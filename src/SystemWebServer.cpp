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
#include <iostream>

// lib
#include <SDL_net.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Defines.hpp"
#include "Helpers.hpp"
#include "Network.hpp"

using namespace std;
using namespace helpers;
using namespace network;

// static functions
static void serverDataRequest(char* cRequest, char* buffer);

// static variables
static TCPsocket client;

void SystemWebServer() {
  IPaddress addr;
  SDLNet_ResolveHost(&addr, nullptr, TCP_WEBSERVER);
  TCPsocket server = SDLNet_TCP_Open(&addr);
  bool& systemOn = Globals::get<bool>("systemOn").value();
  while (systemOn) {
    if ((client = SDLNet_TCP_Accept(server)) != nullptr) {
      char buf[SIZE_WEBSERVER_MAXBUF];
      memset(buf, 0, SIZE_WEBSERVER_MAXBUF);
      SDLNet_TCP_Recv(client, buf, SIZE_WEBSERVER_MAXBUF);
      //printf("------ PACOTE ------------------------------------------\n");
      //printf("\n%s\n", buf);
      fflush(stdout);
      
      char fn[100], buftmp[100];
      sscanf(buf, "%s %s", buftmp, fn);
      //printf("\nFN: %s\n", fn);
      //printf("\nBUF: %s\n", buf);
      

      if (fn[1] == '?') {
    	  serverDataRequest(fn, buf);
      } else {
    	  if (string(fn) == "/")
    		  strcpy(fn, "/index.html");
    	  FILE* fp = fopen((string("./www") + fn).c_str(), "rb");
    	  if (fp) {
    		  size_t total;
    	      while ((total = fread(buf, sizeof(char), SIZE_WEBSERVER_MAXBUF, fp)) > 0)
    	    	  SDLNet_TCP_Send(client, buf, total);
    	      fclose(fp);
    	  }
    	  else {
    		  const char* msg = "<html><body>Pagina nao encontrada.</body></html>";
    		  SDLNet_TCP_Send(client, msg, strlen(msg) + 1);
    	  }
      }
      
      SDLNet_TCP_Close(client);
    }
    Thread::sleep(MS_SLEEP);
  }
  SDLNet_TCP_Close(server);
}

static void serverDataRequest(char* cRequest, char* buffer){
	string request = string(cRequest).substr(3, strlen(cRequest));

	cout << "\nREQUEST: " << request << "\n";

	if (request == "host-ip"){
		printf("\n%s\n", buffer);
		string hostIP = Address(Globals::get<uint32_t>("localIP").value(), 0).toString();
		SDLNet_TCP_Send(client, hostIP.c_str(), hostIP.size() + 1);
	} else if( request == "n-hosts" ){

	} else if( request == "server-state" ){
		SDLNet_TCP_Send(client, "On", 3);
	}
}
