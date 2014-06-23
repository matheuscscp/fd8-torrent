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

// FIXME: esse define eh zoado, tem que tirar
#define SIZE_HTTPSERVER_MAXBUF 0x1000

using namespace std;
using namespace concurrency;
using namespace network;
using namespace helpers;

// static variables
static TCPConnection* client = nullptr;

void System::httpServer() {
  client = httpTCPServer.accept();
  if (client == nullptr)
    return;

  ByteQueue data(SIZE_HTTPSERVER_MAXBUF);
  client->recv(data);
  data.push(char('\0'));
  printf("total bytes request: %d\n%s\n", data.size() - 1, (char*)data.ptr());
  fflush(stdout);
  
  char fn[100], buftmp[100];
  sscanf((char*)data.ptr(), "%s %s", buftmp, fn);

  if (string(fn).find("?") != string::npos) {
    httpServer_dataRequest(fn);
  } else {
    if (string(fn) == "/")
      strcpy(fn, "/index.html");
    FILE* fp = fopen((string("./www") + fn).c_str(), "rb");
    if (fp) {
      if (string(fn).find(".html") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/html\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (string(fn).find(".css") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/css\r\n"
          "\r\n"
        ;
        client->send(header, strlen(header));
      }
      else if (string(fn).find(".js") != string::npos) {
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
      const char* msg = "<html><body>Pagina nao encontrada.</body></html>";
      client->send(msg, strlen(msg) + 1);
    }
  }
  
  delete client;
  client = nullptr;
}

void System::httpServer_dataRequest(char* cRequest) {
  string request = string(cRequest).substr(string(cRequest).find("?") + 1, strlen(cRequest));

  if (request == "host-ip"){
    client->send(localAddress.toString());
  } else if( request == "total-files" ){
    client->send(toString(FileSystem::getTotalFiles()));
  } else if( request == "total-folders" ){
    client->send(toString(FileSystem::getTotalFolders()));
  } else if( request == "total-size" ){
    string totalFiles = toString(FileSystem::getTotalSize());
    client->send(totalFiles);
  } else if( request == "n-hosts" ){
    char tmp[10];
    sprintf(tmp, "%d", users.size());
    client->send(tmp, strlen(tmp) + 1);
  } else if( request == "server-state" ){
    client->send("On", 3);
  } else if( request.find("Cfolder") != string::npos ){
    string tmp = string(request).substr(string(request).find("=") + 1, request.size());
    if(!FileSystem::createFolder(tmp)){
      client->send("0");
    } else {
      client->send("1");
    }
  } else if( request.find("Rfolder") != string::npos ){
    string folderPath = string(request).substr(string(request).find("=") + 1, request.size());
    FileSystem::Folder* folder = FileSystem::retrieveFolder(folderPath);
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
      tableContent += kv.second.getTotalSize();
      tableContent += "</td><td></td><td><a onclick='deleteFolder(\"";
      tableContent += (folderPath == "/") ? kv.first : folderPath + kv.first;
      tableContent += "\")'><img src='img/delete.png'/></a></td></tr>";
    }
    for(auto& kv : folder->files){
      tableContent += "<tr><td><img src='img/folder.png'/></td><td><label onclick='retrieveFile(";
      tableContent += folderPath + kv.first;
      tableContent += ")'>";
      tableContent += kv.first.substr(1, kv.first.size());
      tableContent += "</label></td><td>";
      tableContent += kv.second.size;
      tableContent += "</td><td>";
      tableContent += kv.second.author;
      tableContent += "</td><td><a";
      tableContent += "><img src='img/download.png'/></a></td></tr>";
    }
    client->send(tableContent.c_str(), tableContent.size());
  } else if( request.find("Ufolder") != string::npos ){
  } else if( request.find("Dfolder") != string::npos ){
    if(!FileSystem::deleteFolder(string(request).substr(string(request).find("=") + 1, request.size()))){
      client->send("0");
    } else {
      client->send("1");
    }
  } else if( request.find("Cfile") != string::npos ){
  } else if( request.find("Rfile") != string::npos ){
  } else if( request.find("Ufile") != string::npos ){
  } else if( request.find("Dfile") != string::npos ){
  } else if( request.find("detail-file") != string::npos ){
    string fullPath = string(request).substr(string(request).find("=") + 1, request.size());
    FileSystem::File* file = FileSystem::retrieveFile(fullPath);
    if (!file){
      client->send("0");
      return;
    }
    string json = "{ 'fullPath' : '";
    json += fullPath;
    json += "', 'size' : '";
    json += file->size;
    json += "', 'peer1' : '";
    json += file->peer1;
    json += "', 'peer2' : '";
    json += file->peer2;
    json += "}";
    client->send(json.c_str(), json.size());
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
    client->send(tableContent.c_str(), tableContent.size());
  }
}
