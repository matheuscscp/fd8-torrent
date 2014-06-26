/*
 * System_loginHttpServer.cpp
 *
 *  Created on: Jun 26, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

using namespace std;
using namespace network;
using namespace helpers;

// static variables
static TCPConnection* client = nullptr;

void System::loginHttpServer() {
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
  { // discarding the rest of the request
    ByteQueue tmp(SIZE_HTTPSERVER_MAXLEN);
    client->recv(tmp);
  }
  
  if (requestLine.find("?") != string::npos) {
    loginHttpServer_loginAttempt(requestLine);
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

void System::loginHttpServer_loginAttempt(const string& data) {
  string input = data.substr(data.find("?") + 1, data.size());
  
  if (!input.size())
    return;
  
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
