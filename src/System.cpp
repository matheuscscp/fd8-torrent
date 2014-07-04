/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "FileSystem.hpp"
#include "FD8Protocol.hpp"

using namespace std;
using namespace concurrency;
using namespace network;
using namespace helpers;

Thread* System::thread = nullptr;
bool System::started = false;
bool System::initialized = false;

System::User::User() : sessionID(0) {
  
}

System::User::User(const string& name) : sessionID(0), name(name) {
  
}

bool System::start() {
  if (started | initialized)
    return false;
  started = true;
  thread = new Thread([]() {
    network::init();
    {
      System sys;
      initialized = true;
      sys.run();
    }
    network::close();
    initialized = false;
  });
  thread->start();
  return true;
}

void System::stop(bool wait) {
  if (wait && thread == nullptr) {
    while (initialized)
      Thread::sleep(MS_WAIT);
    return;
  }
  if (!started | !initialized)
    return;
  started = false;
  if (wait)
    thread->join();
  delete thread;
  thread = nullptr;
  return;
}

bool System::changing() {
  return started != initialized;
}

bool System::running() {
  return started && initialized;
}

System::System() :
nextSessionID(1),
state(STATE_NONE),
newState(STATE_LOGIN),
broadcastAddress("255.255.255.255", TCPUDP_MAIN),
multicastAddress(IP_MULTICAST, TCPUDP_MAIN),
mainUDPSocket(multicastAddress, SIZE_UDPSOCKET_MAXLEN),
mainTCPServer(TCPUDP_MAIN),
httpTCPServer(TCP_HTTPSERVER),
httpThread([]() {}),
downloadsRemaining(0)
{
  
}

System::~System() {
  state = STATE_NONE;
  httpThread.join();
  while (downloadsRemaining)
    Thread::sleep(MS_SLEEP);
}

void System::reopenSockets() {
  mainUDPSocket = UDPSocket(multicastAddress, SIZE_UDPSOCKET_MAXLEN);
  mainTCPServer = TCPServer(TCPUDP_MAIN);
  httpTCPServer = TCPServer(TCP_HTTPSERVER);
}

void System::run() {
  while (started) {
    switch (state) {
      case STATE_LOGIN: stateLogin(); break;
      case STATE_IDLE:  stateIdle();  break;
      default:                        break;
    }
    if (newState != state)
      change();
    Thread::sleep(MS_SLEEP);
  }
}

void System::change() {
  httpThread.join();
  state = newState;
  switch (newState) {
    case STATE_LOGIN: changeToLogin();  break;
    case STATE_IDLE:  changeToIdle();   break;
    default:                            break;
  }
}

void System::changeToLogin() {
  users.erase(localAddress.ip);
  localAddress = Address::local();
  reopenSockets();
  loginSyncTimer.start();
  httpThread = Thread([this]() {
    while (state == newState) {
      loginHttpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
}

void System::stateLogin() {
  listen();
  detectFailure();
}

void System::changeToIdle() {
  localAddress = Address::local();
  reopenSockets();
  FileSystem::init(localAddress.ip);
  requestSystemState();
  idleBalancingTimer.start();
  httpThread = Thread([this]() {
    while (state == newState) {
      httpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
  downloadsRemaining = 0;
}

void System::stateIdle() {
  // first balancing of this peer
  if (idleBalancingTimer.time() > MS_WAITRECOVERY) {
    idleBalancingTimer.reset();
    
    // ask for synchronization
    {
      User& user = users[localAddress.ip];
      uint32_t mySessionID = user.sessionID;
      uint32_t tmpNextSessionID = nextSessionID;
      requestSystemState();
      if (nextSessionID != 2) {
        user.sessionID = mySessionID;
        nextSessionID = tmpNextSessionID;
      }
    }
    
    set<uint32_t> peers;
    for (auto& kv : users)
      peers.insert(kv.first);
    list<FileSystem::Command*> cmds;
    FileSystem::initTmpFileSystem();
    if (users.size() == 2)
      cmds = FileSystem::calculateDuplications(peers);
    else
      cmds = FileSystem::calculateBalancing(peers);
    ByteQueue data = FileSystem::Command::serialize(cmds);
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::COMMANDS));
      conn.send(uint32_t(data.size()));
      conn.send(data);
    }
    send_files(cmds);
    FileSystem::processCommands(cmds);
    for (auto& cmd : cmds)
      delete cmd;
  }
  
  executeProtocol();
  speak();
  listen();
  detectFailure();
}

void System::executeProtocol() {
  TCPConnection* peer = mainTCPServer.accept();
  if (peer == nullptr)
    return;
  
  char request = peer->recv<char>();
  switch (request) {
    case fd8protocol::SYNC:
      respondSystemState(peer);
      break;
      
    case fd8protocol::CREATE_FOLDER:
      recv_createFolder(peer->recv<string>());
      break;
      
    case fd8protocol::UPDATE_FOLDER:
      {
        string fullPath = peer->recv<string>();
        recv_updateFolder(fullPath, peer->recv<string>());
      }
      break;
      
    case fd8protocol::DELETE_FOLDER:
      recv_deleteFolder(peer->recv<string>());
      break;
      
    case fd8protocol::CREATE_FILE:
      {
        string fullPath = peer->recv<string>();
        ByteQueue info(peer->recv<uint32_t>());
        peer->recv(info);
        recv_createFile(fullPath, info);
      }
      break;
      
    case fd8protocol::UPDATE_FILE:
      {
        string fullPath = peer->recv<string>();
        recv_updateFile(fullPath, peer->recv<string>());
      }
      break;
      
    case fd8protocol::DELETE_FILE:
      recv_deleteFile(peer->recv<string>());
      break;
      
    case fd8protocol::COMMANDS:
      {
        ByteQueue data(peer->recv<uint32_t>());
        peer->recv(data);
        list<FileSystem::Command*> cmds = FileSystem::Command::deserialize(data);
        send_files(cmds);
        FileSystem::processCommands(cmds);
        for (auto& cmd : cmds)
          delete cmd;
      }
      break;
      
    case fd8protocol::FILE:
      {
        TCPConnection* tmpConn = peer;
        peer = nullptr;
        Thread([tmpConn]() {
          uint32_t fileID = tmpConn->recv<uint32_t>();
          uint32_t fileSize = tmpConn->recv<uint32_t>();
          
          // opening file
          char tmp[25];
          sprintf(tmp, "www/files/%08x", fileID);
          FILE* fp = fopen(tmp, "wb");
          
          // receiving file
          size_t bytesRecvd = 0;
          ByteQueue buf;
          while (bytesRecvd < fileSize) {
            size_t diff = fileSize - bytesRecvd;
            buf.resize(SIZE_FILEBUFFER_MAXLEN < diff ? SIZE_FILEBUFFER_MAXLEN : diff);
            tmpConn->recv(buf);
            bytesRecvd += buf.size();
            fwrite(buf.ptr(), buf.size(), 1, fp);
          }
          
          fclose(fp);
          
          delete tmpConn;
        }).start();
      }
      break;
      
    default:
      break;
  }
  
  if(peer)
    delete peer;
}

void System::speak() {
  static Timer timer;
  
  if (!timer.counting())
    timer.start();
  if (timer.time() > MS_SPEAK) {
    User& user = users[localAddress.ip];
    ByteQueue data;
    data.push(user.sessionID).push(user.name);
    mainUDPSocket.send(broadcastAddress, data);
    mainUDPSocket.send(multicastAddress, data);
    timer.start();
  }
}

void System::listen() {
  // listening
  Address addr;
  ByteQueue data;
  data = mainUDPSocket.recv(addr);
  if (!data.size()) // no beacon was received
    return;
  
  uint32_t sessionID = data.pop<uint32_t>();
  string userName = data.pop<string>();
  
  // ignoring beacons sent from this peer and non-private network addresses
  if (userName == users[localAddress.ip].name || !addr.isPrivateNetwork())
    return;
  
  auto userIt = users.find(addr.ip);
  if (userIt != users.end()) { // IP already in the user table
    if (userIt->second.sessionID == sessionID) // refresh beacon
      userIt->second.timer.start();
    else { // someone logged out and then logged in, failure detected
      users.erase(userIt);
      if (state == STATE_IDLE)
        recoverFromFailure();
      User& user = users[addr.ip];
      user.sessionID = sessionID;
      user.name = userName;
      user.timer.start();
      nextSessionID = user.sessionID + 1;
    }
  }
  else { // IP just logged in
    User& user = users[addr.ip];
    user.sessionID = sessionID;
    user.name = userName;
    user.timer.start();
    nextSessionID = user.sessionID + 1;
  }
}

void System::detectFailure() {
  bool failureDetected = false;
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE) {
      users.erase(it++);
      failureDetected = true;
    }
    else
      ++it;
  }
  
  if (failureDetected && state == STATE_IDLE)
    recoverFromFailure();
}

void System::recoverFromFailure() {
  // check who is the designated peer
  set<uint32_t> peers;
  uint32_t designatedPeer = 0;
  for (auto& kv : users) {
    peers.insert(kv.first);
    if (kv.first > designatedPeer)
      designatedPeer = kv.first;
  }
  
  // remains only this peer
  if (peers.size() == 1) {
    string zuera;
    FileSystem::retrieveFolder("/", zuera)->removeOfflinePeers(peers);
    return;
  }
  
  // if the designated peer is not this one
  if (designatedPeer != localAddress.ip)
    return;
  
  // recover
  FileSystem::initTmpFileSystem();
  list<FileSystem::Command*> cmds = FileSystem::calculateDuplications(peers);
  list<FileSystem::Command*> balCmds = FileSystem::calculateBalancing(peers);
  for (auto& cmd : balCmds)
    cmds.push_back(cmd);
  FileSystem::eliminateIntersections(cmds);
  ByteQueue data = FileSystem::Command::serialize(cmds);
  for (auto& kv : users) {
    if (kv.first == localAddress.ip)
      continue;
    TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
    conn.send(char(fd8protocol::COMMANDS));
    conn.send(uint32_t(data.size()));
    conn.send(data);
  }
  send_files(cmds);
  FileSystem::processCommands(cmds);
  for (auto& cmd : cmds)
    delete cmd;
}

void System::loginHttpServer() {
  TCPConnection* client = httpTCPServer.accept();
  if (client == nullptr)
    return;
  
  string requestLine;
  // check dumb requests
  {
    char c;
    if (!client->recv(&c, 1)) {
      delete client;
      client = nullptr;
      return;
    }
    else
      requestLine += c;
  }
  for (char c; (c = client->recv<char>()) != '\n'; requestLine += c); // receive the request line
  for (; requestLine[0] != ' '; requestLine = requestLine.substr(1, requestLine.size())); // remove method
  requestLine = requestLine.substr(1, requestLine.size()); // remove space after method
  for (; requestLine[requestLine.size() - 1] != ' '; requestLine = requestLine.substr(0, requestLine.size() - 1)); // remove http version
  requestLine = requestLine.substr(0, requestLine.size() - 1);// remove space before http version
  { // discarding the rest of the request
    ByteQueue tmp(SIZE_HTTPSERVER_MAXLEN);
    client->recv(tmp);
  }
  
  if (requestLine.find("?login") != string::npos) {
    loginHttpServer_loginAttempt(client, requestLine);
  }
  else {
    if (requestLine == "/" || requestLine == "/index.html")
      requestLine = "/login.html";
    FILE* fp = fopen((string("./www") + requestLine).c_str(), "rb");
    if (fp) {
      if (requestLine.find(".html") != string::npos) {
        const char* header =
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/html\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (requestLine.find(".css") != string::npos) {
        const char* header =
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/css\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (requestLine.find(".js") != string::npos) {
        const char* header =
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/javascript\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else {
        const char* header =
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/octet-stream\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      client->send(fp);
      fclose(fp);
    }
    else {
      const char* msg =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\r"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<html><body>Pagina nao encontrada.</body></html>"
      ;
      client->send(msg, strlen(msg) + 1);
    }
  }
  
  delete client;
  client = nullptr;
}

void System::loginHttpServer_loginAttempt(TCPConnection* client, const string& request) {
  if (loginSyncTimer.time() < MS_LOGINSYNC) {
    client->send(string("2"), true);
    return;
  }
  
  string input = request.substr(request.find("=") + 1, request.size());
  
  if (!input.size()) {
    client->send(string("0"), true);
    return;
  }
  
  for(auto& kv : users) {
    if(kv.second.name == input){
      client->send(string("0"), true);
      return;
    }
  }
  
  users[localAddress.ip] = User(input);
  client->send(string("1"), true);
  newState = STATE_IDLE;
}

void System::httpServer() {
  TCPConnection* client = httpTCPServer.accept();
  if (client == nullptr)
    return;
  
  string requestLine;
  // check dumb requests
  {
    char c;
    if (!client->recv(&c, 1)) {
      delete client;
      client = nullptr;
      return;
    }
    else
      requestLine += c;
  }
  for (char c; (c = client->recv<char>()) != '\n'; requestLine += c); // receive the request line
  for (; requestLine[0] != ' '; requestLine = requestLine.substr(1, requestLine.size())); // remove method
  requestLine = requestLine.substr(1, requestLine.size()); // remove space after method
  for (; requestLine[requestLine.size() - 1] != ' '; requestLine = requestLine.substr(0, requestLine.size() - 1)); // remove http version
  requestLine = requestLine.substr(0, requestLine.size() - 1);// remove space before http version
  if (requestLine.find("Cfile") != string::npos)
    httpServer_recvFile(client);
  else { // discarding the rest of the request
    ByteQueue tmp(SIZE_HTTPSERVER_MAXLEN);
    client->recv(tmp);
  }
  
  if (requestLine.find("?") != string::npos) {
    httpServer_dataRequest(client, requestLine);
  } else {
    if (requestLine == "/" || requestLine == "/login.html")
      requestLine = "/index.html";
    FILE* fp = fopen((string("./www") + requestLine).c_str(), "rb");
    if (fp) {
      if (requestLine.find(".html") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/html\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (requestLine.find(".css") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/css\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (requestLine.find(".js") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/javascript\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/octet-stream\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      client->send(fp);
      fclose(fp);
    }
    else {
      const char* msg =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\r"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<html><body>Pagina nao encontrada.</body></html>"
      ;
      client->send(msg, strlen(msg) + 1);
    }
  }
  
  if (client)
    delete client;
  client = nullptr;
}

void System::httpServer_dataRequest(TCPConnection* client, const string& request) {
  string req = request.substr(request.find("?") + 1, request.size());
  if (req == "logout"){
    client->send(string("1"), true);
    newState = STATE_LOGIN;
  } else if (req == "host-ip"){
    client->send(localAddress.toString());
  } else if( req == "total-files" ){
    client->send(toString(FileSystem::getTotalFiles()));
  } else if( req == "username" ){
    client->send(users[localAddress.ip].name);
  } else if( req == "total-folders" ){
    client->send(toString(FileSystem::getTotalFolders()));
  } else if( req == "total-size" ){
    client->send(toString(FileSystem::getTotalSize()));
  } else if( req == "n-hosts" ){
    client->send(toString(users.size()));
  } else if( req == "server-state" ){
    client->send("1");
  } else if( req.find("folder-tfolders") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(req.substr(req.find("=") + 1, req.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalFolders()));
  } else if( req.find("folder-tfiles") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(req.substr(req.find("=") + 1, req.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalFiles()));
  } else if( req.find("folder-tsize") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(req.substr(req.find("=") + 1, req.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalSize()));
  } else if( req.find("Cfolder") != string::npos ){
    string tmp = req.substr(req.find("=") + 1, req.size());
    if(!FileSystem::createFolder(tmp)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_createFolder(tmp);
    }
  } else if( req.find("RfolderPath") != string::npos ){
    string folderPath = req.substr(req.find("=") + 1, req.size());
    string foundPath;
    FileSystem::retrieveFolder(folderPath, foundPath);
    client->send(foundPath, true);
  } else if( req.find("Rfolder") != string::npos ){
    string folderPath = req.substr(req.find("=") + 1, req.size());
    string foundPath;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(folderPath, foundPath);
    if (!folder){
      client->send("0");
      return;
    }
    string tableContent;
    for(auto& kv : folder->subfolders) {
      tableContent += "<tr><td><img src='img/folder.png'/></td><td><label onclick='retrieveFolder(\"";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\")'>";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "</label></td><td>";
      tableContent += toString(kv.second.getTotalSize());
      tableContent += "</td><td></td><td><a onclick='editFolder(\"";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "\")'><img src='img/edit.png'/></a><a onclick='deleteFolder(\"";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "\")'><img src='img/delete.png'/></a></td></tr>";
    }
    for(auto& kv : folder->files){
      tableContent += "<tr><td><img src='img/fileimg.png'/></td><td><label onclick='retrieveFile(";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += ")'>";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "</label></td><td>";
      tableContent += toString(kv.second.size);
      tableContent += "</td><td>";
      tableContent += kv.second.author;
      tableContent += "</td><td><a onclick='editFile(\"";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "\")'><img src='img/edit.png'/></a><a onclick='deleteFile(\"";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\")'><img src='img/delete.png'/></a><a href=\"http://";
      tableContent += Address(kv.second.peer1, Address("", TCP_HTTPSERVER).port).toString();
      tableContent += "/?Rfile=";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\" download=\"";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "\"><img src='img/download.png'/></a><a href=\"http://";
      tableContent += Address(kv.second.peer2, Address("", TCP_HTTPSERVER).port).toString();
      tableContent += "/?Rfile=";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\" download=\"";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "\"><img src='img/download.png'/></a></td></tr>";
    }
    client->send(tableContent);
  } else if( req.find("Ufolder") != string::npos ){
    string data = req.substr(req.find("=") + 1, req.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFolder(oldPath, newName))
      client->send("0");
    else {
      client->send(string("1"), true);
      send_updateFolder(oldPath, newName);
    }
  } else if( req.find("Dfolder") != string::npos ){
    string tmp = string(req).substr(string(req).find("=") + 1, req.size());
    if(!FileSystem::deleteFolder(tmp)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_deleteFolder(tmp);
    }
  } else if( req.find("Cfile") != string::npos ){
    string fullPath = req.substr(req.find("=") + 1, req.size());
    FileSystem::File* file = FileSystem::createFile(fullPath, users[localAddress.ip].name);
    if(!file){
      client->send("0");
    } else {
      client->send(string("1"), true);
      ByteQueue info;
      file->serialize(info);
      send_createFile(fullPath, info);
    }
  } else if( req.find("Rfile") != string::npos ){
    FileSystem::File* file = FileSystem::retrieveFile(string(req).substr(string(req).find("=") + 1, req.size()));
    if (file) {
      char tmp[25];
      sprintf(tmp, "www/files/%08x", file->id);
      FILE* fp = fopen(tmp, "rb");
      if (!fp)
        return;
      TCPConnection* tmpConn = client;
      client = nullptr;
      Thread([this, fp, tmpConn]() {
        downloadsRemaining++;
        char buf[SIZE_FILEBUFFER_MAXLEN];
        for (
          size_t readBytes;
          (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
          tmpConn->send(buf, readBytes)
        );
        delete tmpConn;
        fclose(fp);
        downloadsRemaining--;
      }).start();
    }
  } else if( req.find("Ufile") != string::npos ){
    string data = req.substr(req.find("=") + 1, req.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFile(oldPath, newName))
      client->send("0");
    else {
      client->send(string("1"), true);
      send_updateFile(oldPath, newName);
    }
  } else if( req.find("Dfile") != string::npos ){
    string fullPath = string(req).substr(string(req).find("=") + 1, req.size());
    if(!FileSystem::deleteFile(fullPath)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_deleteFile(fullPath);
    }
  } else if( req == "list-users" ){
    string tableContent;
    for(auto& kv : users) {
      tableContent += "<tr><td><img src='img/userimg.png'/></td><td>";
      tableContent += kv.second.name;
      tableContent += "</td>";
      tableContent += "<td>";
      tableContent += Address(kv.first, 0).toString();
      tableContent += "</td></tr>";
    }
    client->send(tableContent);
  }
}

void System::httpServer_recvFile(TCPConnection* client) {
  size_t fileSize;
  
  string tmp;
  while(true){
    tmp = "";
    for (char c; (c = client->recv<char>()) != '\n'; tmp += c);
    if(tmp.find("Tamanho") != string::npos){
      fileSize = fromString<size_t>(tmp.substr(tmp.find(":") + 1, tmp.size() - 2).c_str());
    } else if (tmp == "\r"){
      break;
    }
  }
  while(true){
    tmp = "";
    for (char c; (c = client->recv<char>()) != '\n'; tmp += c);
    if (tmp == "\r"){
      break;
    }
  }
  
  size_t bytesRecvd = 0;
  ByteQueue buf;
  FILE* fp = fopen("www/files/tmp", "wb");
  while (bytesRecvd < fileSize) {
    size_t diff = fileSize - bytesRecvd;
    buf.resize(SIZE_FILEBUFFER_MAXLEN < diff ? SIZE_FILEBUFFER_MAXLEN : diff);
    client->recv(buf);
    bytesRecvd += buf.size();
    fwrite(buf.ptr(), buf.size(), 1, fp);
  }
  fclose(fp);
  client->recv<char>();
  client->recv<char>();
  for (char c; (c = client->recv<char>()) != '\n';);
}

void System::requestSystemState() {
  // find someone to send the system state to this peer
  uint32_t ip = 0;
  for (auto& kv : users) {
    if (kv.first != localAddress.ip) {
      ip = kv.first;
      break;
    }
  }
  
  User& user = users[localAddress.ip];
  
  if (!ip) { // no one found, then return
    nextSessionID = 1;
    user.sessionID = nextSessionID++;
    return;
  }
  
  // ask for synchronization
  TCPConnection conn(Address(ip, Address("", TCPUDP_MAIN).port));
  conn.send(char(fd8protocol::SYNC));
  user.sessionID = conn.recv<uint32_t>();
  nextSessionID = user.sessionID + 1;
  ByteQueue data(conn.recv<uint32_t>());
  conn.recv(data);
  FileSystem::deserialize(data);
}

void System::respondSystemState(TCPConnection* peer) {
  peer->send(nextSessionID);
  ByteQueue data = FileSystem::serialize();
  peer->send(uint32_t(data.size()));
  peer->send(data);
}

void System::send_createFolder(const string& fullPath) {
  Thread([this, fullPath]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::CREATE_FOLDER));
      conn.send(fullPath);
    }
  }).start();
}

void System::send_updateFolder(const string& fullPath, const string& newName) {
  Thread([this, fullPath, newName]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::UPDATE_FOLDER));
      conn.send(fullPath);
      conn.send(newName);
    }
  }).start();
}

void System::send_deleteFolder(const string& fullPath) {
  Thread([this, fullPath]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::DELETE_FOLDER));
      conn.send(fullPath);
    }
  }).start();
}

void System::recv_createFolder(const string& fullPath) {
  FileSystem::createFolder(fullPath);
}

void System::recv_updateFolder(const string& fullPath, const string& newName) {
  FileSystem::updateFolder(fullPath, newName);
}

void System::recv_deleteFolder(const string& fullPath) {
  FileSystem::deleteFolder(fullPath);
}

void System::send_createFile(const string& fullPath, const ByteQueue& info) {
  Thread([this, fullPath, info]() {
    set<uint32_t> peers;
    
    for (auto& kv : users) {
      peers.insert(kv.first);
      
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::CREATE_FILE));
      conn.send(fullPath);
      conn.send(uint32_t(info.size()));
      conn.send(info);
    }
    
    FileSystem::initTmpFileSystem();
    list<FileSystem::Command*> cmds = FileSystem::calculateDuplications(peers);
    list<FileSystem::Command*> balCmds = FileSystem::calculateBalancing(peers);
    for (auto& cmd : balCmds)
      cmds.push_back(cmd);
    FileSystem::eliminateIntersections(cmds);
    ByteQueue data = FileSystem::Command::serialize(cmds);
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::COMMANDS));
      conn.send(uint32_t(data.size()));
      conn.send(data);
    }
    
    send_files(cmds);
    FileSystem::processCommands(cmds);
    
    for (auto& cmd : cmds)
      delete cmd;
  }).start();
}

void System::send_updateFile(const string& fullPath, const string& newName) {
  Thread([this, fullPath, newName]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::UPDATE_FILE));
      conn.send(fullPath);
      conn.send(newName);
    }
  }).start();
}

void System::send_deleteFile(const string& fullPath) {
  Thread([this, fullPath]() {
    set<uint32_t> peers;
    
    for (auto& kv : users) {
      peers.insert(kv.first);
      
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::DELETE_FILE));
      conn.send(fullPath);
    }
    
    FileSystem::initTmpFileSystem();
    list<FileSystem::Command*> cmds = FileSystem::calculateBalancing(peers);
    ByteQueue data = FileSystem::Command::serialize(cmds);
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(fd8protocol::COMMANDS));
      conn.send(uint32_t(data.size()));
      conn.send(data);
    }
    
    send_files(cmds);
    FileSystem::processCommands(cmds);
    
    for (auto& cmd : cmds)
      delete cmd;
  }).start();
}

void System::recv_createFile(const string& fullPath, ByteQueue& info) {
  FileSystem::createFile(fullPath, info);
}

void System::recv_updateFile(const string& fullPath, const string& newName) {
  FileSystem::updateFile(fullPath, newName);
}

void System::recv_deleteFile(const string& fullPath) {
  FileSystem::deleteFile(fullPath);
}

void System::send_files(const list<FileSystem::Command*>& cmds) {
  for (auto& cmd : cmds) {
    if(cmd->type() == FileSystem::Command::DUPLICATION && ((FileSystem::DuplicationCommand*)cmd)->srcPeer == localAddress.ip) {
      FileSystem::DuplicationCommand dupCmd = *((FileSystem::DuplicationCommand*)cmd);
      Thread([this, dupCmd]() {
        char buf[SIZE_FILEBUFFER_MAXLEN];
        
        // getting file name
        char tmp[25];
        sprintf(tmp, "www/files/%08x", dupCmd.fileID);
        
        FILE* fp = fopen(tmp, "rb");
        fseek(fp, 0, SEEK_END);
        
        TCPConnection conn(Address(dupCmd.dstPeer, Address("", TCPUDP_MAIN).port));
        conn.send(char(fd8protocol::FILE));
        conn.send(uint32_t(dupCmd.fileID));
        conn.send(uint32_t(ftell(fp)));
        fclose(fp);
        fp = fopen(tmp, "rb");
        for (
          size_t readBytes;
          (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
          conn.send(buf, readBytes)
        );
        fclose(fp);
      }).start();
    }
    else if (cmd->type() == FileSystem::Command::BALANCING) {
      FileSystem::BalancingCommand balCmd = *((FileSystem::BalancingCommand*)cmd);
      string zuera;
      FileSystem::File file = *FileSystem::retrieveFolder("/", zuera)->findFile(balCmd.fileID);
      if (balCmd.srcPeer == localAddress.ip) {
        Thread([this, balCmd, file]() {
          char buf[SIZE_FILEBUFFER_MAXLEN];
          
          // getting file name
          char tmp[25];
          sprintf(tmp, "www/files/%08x", balCmd.fileID);
          
          // getting file size
          FILE* fp = fopen(tmp, "rb");
          fseek(fp, 0, SEEK_END);
          uint32_t fileSize = ftell(fp);
          fclose(fp);
          
          // sending to peer 1
          if (balCmd.peer1 != file.peer1 && balCmd.peer1 != file.peer2) {
            TCPConnection conn(Address(balCmd.peer1, Address("", TCPUDP_MAIN).port));
            conn.send(char(fd8protocol::FILE));
            conn.send(uint32_t(balCmd.fileID));
            conn.send(fileSize);
            fp = fopen(tmp, "rb");
            for (
              size_t readBytes;
              (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
              conn.send(buf, readBytes)
            );
            fclose(fp);
          }
          
          // sending to peer 2
          if (balCmd.peer2 != file.peer1 && balCmd.peer2 != file.peer2) {
            TCPConnection conn(Address(balCmd.peer2, Address("", TCPUDP_MAIN).port));
            conn.send(char(fd8protocol::FILE));
            conn.send(uint32_t(balCmd.fileID));
            conn.send(fileSize);
            fp = fopen(tmp, "rb");
            for (
              size_t readBytes;
              (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
              conn.send(buf, readBytes)
            );
            fclose(fp);
          }
          
          remove(tmp);
        }).start();
      }
      else if ((file.peer1 == localAddress.ip || file.peer2 == localAddress.ip) && balCmd.peer1 != localAddress.ip && balCmd.peer2 != localAddress.ip) {
        char tmp[25];
        sprintf(tmp, "www/files/%08x", balCmd.fileID);
        remove(tmp);
      }
    }
  }
}
