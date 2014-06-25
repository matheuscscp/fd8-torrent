/*
 * System_stateLogin.cpp
 *
 *  Created on: Jun 19, 2014
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

// static functions
static int loginAttempt(char* data, map<uint32_t, User>& users, uint32_t ip);

void System::changeToLogin() {
  users.clear();
  FileSystem::init(localAddress.ip);
  state = STATE_LOGIN;
}

void System::stateLogin() {
  requestSystemState();
  
  client = httpTCPServer.accept();
  if (client == nullptr)
    return;
  
  ByteQueue data(SIZE_HTTPSERVER_MAXBUF);
  client->recv(data);
  if (!data.size()) // for dumb requests
    return;
  
  char fn[100], buftmp[100];
  sscanf((char*)data.ptr(), "%s %s", buftmp, fn);
  if (fn[1] == '?') {
    if(loginAttempt(fn, users, localAddress.ip))
      changeToIdle();
  }
  else {
    if (string(fn) == "/")
      strcpy(fn, "/login.html");
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

static int loginAttempt(char* data, map<uint32_t, User>& users, uint32_t ip) {
  string input = string(data).substr(string(data).find("?") + 1, strlen(data));
  
  if (!input.size())
    return 0;
  
  for(auto& kv : users) {
    if(kv.second.name == input){
      client->send("0", 2);
      return 0;
    }
  }
  
  users[ip] = User(input);
  client->send("1", 2);
  return 1;
}
