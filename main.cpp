/*
 * main.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: Pimenta
 */

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_main.h>
#include <SDL_net.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

void init() {
  if (SDL_Init(0)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    exit(0);
  }
  
  if (SDLNet_Init()) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(0);
  }
}

void close() {
  SDLNet_Quit();
  SDL_Quit();
}

int main(int argc, char* argv[]) {
  init();
  
  // create a listening TCP socket on port 8080 (server)
  IPaddress ip;
  TCPsocket tcpsock;
  
  if(SDLNet_ResolveHost(&ip,NULL,8080)==-1) {
      printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
      exit(1);
  }
  
  tcpsock=SDLNet_TCP_Open(&ip);
  if(!tcpsock) {
      printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
      exit(2);
  }
  
  char * headers =
      "HTTP 1.1 200 OK\n"
      "Content-Type: text/html;\n"
      "Server: rosettaNaBoletta 0.1;\n"
      "\n";
  char* msg = "<html><body>hello world</body></html>";
  
  while (true) {
    TCPsocket client = SDLNet_TCP_Accept(tcpsock);
    if (client == nullptr)
      continue;
    else
      printf("olha q maravilha\n");
    SDLNet_TCP_Send(client, "HTTP/1.0 200 OK\n\n", 17);
    SDLNet_TCP_Send(client, msg, strlen(msg));
    SDLNet_TCP_Close(client);
  }
  
  SDLNet_TCP_Close(tcpsock);
  
  close();
  
  return 0;
}
