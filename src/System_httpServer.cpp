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
static ByteQueue fileData;

static void recvFile() {
  fileData.resize(0);
  string tmp;

  while(true){
    tmp = "";
    for (char c; (c = client->recv<char>()) != '\n'; tmp += c);
    if(tmp.find("Tamanho") != string::npos){
      fileData.resize(fromString<int>(tmp.substr(tmp.find(":") + 1, tmp.size() - 2).c_str()));
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
  client->recv(fileData);
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
    if (!client->recv(&c, 1)) return;
    else requestLine += c;
  }
  for (char c; (c = client->recv<char>()) != '\n'; requestLine += c); // receive the request line
  for (; requestLine[0] != ' '; requestLine = requestLine.substr(1, requestLine.size())); // remove method
  requestLine = requestLine.substr(1, requestLine.size()); // remove space after method
  for (; requestLine[requestLine.size() - 1] != ' '; requestLine = requestLine.substr(0, requestLine.size() - 1)); // remove http version
  requestLine = requestLine.substr(0, requestLine.size() - 1);// remove space before http version
  if (requestLine.find("Cfile") != string::npos)
    recvFile();
  else { // if the request is NOT for file upload
    fileData.resize(SIZE_HTTPSERVER_MAXBUF);
    client->recv(fileData); // actually, this is the request body... discarding
  }
  
  if (requestLine.find("?") != string::npos) {
    httpServer_dataRequest(requestLine);
  } else {
    if (requestLine == "/")
    requestLine += "/index.html";
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
      client->send(FileSystem::readFile(fp));
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

void System::httpServer_dataRequest(const string& cRequest) {
  string request = cRequest.substr(cRequest.find("?") + 1, cRequest.size());
  if (request == "host-ip"){
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
  } else if( request.find("Cfolder") != string::npos ){
    string tmp = request.substr(request.find("=") + 1, request.size());
    if(!FileSystem::createFolder(tmp)){
      client->send("0");
    } else {
      client->send("1");
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
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\")'><img src='img/delete.png'/></a></td></tr>";
    }
    for(auto& kv : folder->files){
      tableContent += "<tr><td><img src='img/fileimg.png'/></td><td><label onclick='retrieveFile(";
      tableContent += folderPath + kv.first;
      tableContent += ")'>";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "</label></td><td>";
      tableContent += toString(kv.second.size);
      tableContent += "</td><td>";
      tableContent += kv.second.author;
      tableContent += "</td><td><a onclick='editFile(\"";
      tableContent += folderPath + kv.first;
      tableContent += "\")'><img src='img/edit.png'/></a><a onclick='deleteFile(\"";
      tableContent += folderPath + kv.first;
      tableContent += "\")'><img src='img/delete.png'/></a><a";
      tableContent += "><img src='img/download.png'/></a></td></tr>";
    }
    client->send(tableContent);
  } else if( request.find("Ufolder") != string::npos ){
    string data = request.substr(request.find("=") + 1, request.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFolder(oldPath, newName))
      client->send("0");
    else {
      client->send("1");
      send_updateFolder(oldPath, newName);
    }
  } else if( request.find("Dfolder") != string::npos ){
    string tmp = string(request).substr(string(request).find("=") + 1, request.size());
    if(!FileSystem::deleteFolder(tmp)){
      client->send("0");
    } else {
      client->send("1");
      send_deleteFolder(tmp);
    }
  } else if( request.find("Cfile") != string::npos ){
    string fullPath = request.substr(request.find("=") + 1, request.size());
    if(!FileSystem::createFile(fullPath, fileData, users[localAddress.ip].name)){
      client->send("0");
    } else {
      client->send("1");
    }
  } else if( request.find("Ufile") != string::npos ){
    string data = request.substr(request.find("=") + 1, request.size());
    string oldPath = data.substr(0, data.find("?&"));
    string newName = data.substr(data.find("?&") + 2, data.size());
    if(!FileSystem::updateFile(oldPath, newName))
      client->send("0");
    else {
      client->send("1");
      send_updateFolder(oldPath, newName);
    }
  } else if( request.find("Dfile") != string::npos ){
    string fullPath = string(request).substr(string(request).find("=") + 1, request.size());
    if(!FileSystem::deleteFile(fullPath)){
      client->send("0");
    } else {
      client->send("1");
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
