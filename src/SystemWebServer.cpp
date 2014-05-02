/*
 * SystemWeb.cpp
 *
 *  Created on: May 1, 2014
 *      Author: Pimenta
 */

// this
#include "SystemWebServer.hpp"

// standard
#include <cstring>
#include <cstdio>
#include <string>

// lib
#include <SDL_net.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Define.hpp"

using namespace std;

void SystemWebServer() {
  IPaddress addr;
  SDLNet_ResolveHost(&addr, nullptr, TCP_WEB_HOST);
  TCPsocket server = SDLNet_TCP_Open(&addr), client;
  bool& systemOn = Globals::get<bool>("systemOn").value();
  while (systemOn) {
    if ((client = SDLNet_TCP_Accept(server)) != nullptr) {
      char buf[HTTP_MAX_BUFFER_SIZE];
      memset(buf, 0, HTTP_MAX_BUFFER_SIZE);
      SDLNet_TCP_Recv(client, buf, HTTP_MAX_BUFFER_SIZE);
      printf("%s\n", buf);
      fflush(stdout);
      
      char fn[100], buftmp[100];
      sscanf(buf, "%s %s", buftmp, fn);
      if (string(fn) == "/")
        strcpy(fn, "/index.html");
      FILE* fp = fopen((string("./www") + fn).c_str(), "rb");
      if (fp) {
        size_t total;
        while ((total = fread(buf, sizeof(char), HTTP_MAX_BUFFER_SIZE, fp)) > 0)
          SDLNet_TCP_Send(client, buf, total);
        fclose(fp);
      }
      else {
        const char* msg = "<html><body>Pagina nao encontrada.</body></html>";
        SDLNet_TCP_Send(client, msg, strlen(msg) + 1);
      }
      
      SDLNet_TCP_Close(client);
    }
    Thread::sleep(50);
  }
  SDLNet_TCP_Close(server);
}
