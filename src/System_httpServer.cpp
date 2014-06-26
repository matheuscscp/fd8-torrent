/*
 * System_httpServer.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <cstring>
#include <cstdio>
#include <string>

// local
#include "Defines.hpp"
#include "Concurrency.hpp"
#include "Network.hpp"
#include "Helpers.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace concurrency;
using namespace network;
using namespace helpers;

// static variables
static TCPConnection* client = nullptr;

static void recvFile() {
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

void System::httpServer() {
  client = httpTCPServer.accept();
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
    recvFile();
  else { // discarding the rest of the request
    ByteQueue tmp(SIZE_HTTPSERVER_MAXLEN);
    client->recv(tmp);
  }
  
  if (requestLine.find("?") != string::npos) {
    httpServer_dataRequest(requestLine);
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

void System::httpServer_dataRequest(const string& cRequest) {
  string request = cRequest.substr(cRequest.find("?") + 1, cRequest.size());
  if (request == "logout"){
    client->send(string("1"), true);
    newState = STATE_LOGIN;
  } else if (request == "host-ip"){
    client->send(localAddress.toString());
  } else if( request == "total-files" ){
    client->send(toString(FileSystem::getTotalFiles()));
  } else if( request == "username" ){
    client->send(users[localAddress.ip].name);
  } else if( request == "total-folders" ){
    client->send(toString(FileSystem::getTotalFolders()));
  } else if( request == "total-size" ){
    client->send(toString(FileSystem::getTotalSize()));
  } else if( request == "n-hosts" ){
    client->send(toString(users.size()));
  } else if( request == "server-state" ){
    client->send("1");
  } else if( request.find("folder-tfolders") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(request.substr(request.find("=") + 1, request.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalFolders()));
  } else if( request.find("folder-tfiles") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(request.substr(request.find("=") + 1, request.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalFiles()));
  } else if( request.find("folder-tsize") != string::npos ){
    string tmp;
    FileSystem::Folder* folder = FileSystem::retrieveFolder(request.substr(request.find("=") + 1, request.size()), tmp);
    if (folder)
      client->send(toString(folder->getTotalSize()));
  } else if( request.find("Cfolder") != string::npos ){
    string tmp = request.substr(request.find("=") + 1, request.size());
    if(!FileSystem::createFolder(tmp)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_createFolder(tmp);
    }
  } else if( request.find("RfolderPath") != string::npos ){
    string folderPath = request.substr(request.find("=") + 1, request.size());
    string foundPath;
    FileSystem::retrieveFolder(folderPath, foundPath);
    client->send(foundPath, true);
  } else if( request.find("Rfolder") != string::npos ){
    string folderPath = request.substr(request.find("=") + 1, request.size());
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
  } else if( request.find("Ufolder") != string::npos ){
    string data = request.substr(request.find("=") + 1, request.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFolder(oldPath, newName))
      client->send("0");
    else {
      client->send(string("1"), true);
      send_updateFolder(oldPath, newName);
    }
  } else if( request.find("Dfolder") != string::npos ){
    string tmp = string(request).substr(string(request).find("=") + 1, request.size());
    if(!FileSystem::deleteFolder(tmp)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_deleteFolder(tmp);
    }
  } else if( request.find("Cfile") != string::npos ){
    string fullPath = request.substr(request.find("=") + 1, request.size());
    FileSystem::File* file = FileSystem::createFile(fullPath, users[localAddress.ip].name);
    if(!file){
      client->send("0");
    } else {
      client->send(string("1"), true);
      ByteQueue info;
      file->serialize(info);
      send_createFile(fullPath, info);
    }
  } else if( request.find("Rfile") != string::npos ){
    FileSystem::File* file = FileSystem::retrieveFile(string(request).substr(string(request).find("=") + 1, request.size()));
    if (file) {
      char tmp[25];
      sprintf(tmp, "www/files/%08x", file->id);
      FILE* fp = fopen(tmp, "rb");
      if (!fp)
        return;
      TCPConnection* tmpConn = client;
      client = nullptr;
      Thread([fp, tmpConn, this]() {
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
  } else if( request.find("Ufile") != string::npos ){
    string data = request.substr(request.find("=") + 1, request.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFile(oldPath, newName))
      client->send("0");
    else {
      client->send(string("1"), true);
      send_updateFile(oldPath, newName);
    }
  } else if( request.find("Dfile") != string::npos ){
    string fullPath = string(request).substr(string(request).find("=") + 1, request.size());
    if(!FileSystem::deleteFile(fullPath)){
      client->send("0");
    } else {
      client->send(string("1"), true);
      send_deleteFile(fullPath);
    }
  } else if( request == "list-users" ){
    string tableContent;
    for(auto& kv : users) {
      tableContent += "<tr><td>";
      tableContent += kv.second.name;
      tableContent += "</td>";
      tableContent += "<td>";
      tableContent += Address(kv.first, 0).toString();
      tableContent += "</td></tr>";
    }
    client->send(tableContent);
  }
}
