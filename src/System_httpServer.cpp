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

// FIXME: esse define eh zoado, tem que tirar
#define SIZE_HTTPSERVER_MAXBUF 0x1000

using namespace std;
using namespace concurrency;
using namespace network;
using namespace helpers;

// static functions
static void dataRequest(char* cRequest, const string& hostIP, int nUsers);

// static variables
static TCPConnection* client = nullptr;

void System::httpServer() {
  client = httpTCPServer.accept();
  if (client == nullptr)
    return;
  
  vector<char> data = client->recv(SIZE_HTTPSERVER_MAXBUF);
  data.push_back(0);
  printf("total bytes request: %d\n%s\n", data.size(), &data[0]);
  fflush(stdout);
  
  char fn[100], buftmp[100];
  sscanf(&data[0], "%s %s", buftmp, fn);
  if (string(fn).find("?") != string::npos) {
    dataRequest(fn, localAddress.toString(), users.size());
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
      client->send(readFile(fp));
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

static void dataRequest(char* cRequest, const string& hostIP, int nUsers) {
  string request = string(cRequest).substr(string(cRequest).find("?") + 2, strlen(cRequest));

  if (request == "host-ip"){
    client->send(hostIP.c_str(), hostIP.size() + 1);
  } else if( request == "n-hosts" ){
    char tmp[10];
    sprintf(tmp, "%d", nUsers);
    client->send(tmp, strlen(tmp) + 1);
  } else if( request == "server-state" ){
    client->send("On", 3);
  }
}
