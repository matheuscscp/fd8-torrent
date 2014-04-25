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
#include <SDL_timer.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

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
  
  while (true) {
    SDL_Delay(50);
    TCPsocket client = SDLNet_TCP_Accept(tcpsock);
    if (client == nullptr)
      continue;
    
    char buf[9999];
    SDLNet_TCP_Recv(client, buf, 9999);
    printf("===============\nsize: %d\n===============\n%s\n\n", strlen(buf), buf);
    
    char fn[100], buftmp[100];
    sscanf(buf, "%s %s", buftmp, fn);
    if (string(fn) == "/")
      strcpy(fn, "/index.html");
    FILE* fp = fopen((string(".") + fn).c_str(), "rb");
    if (fp) {
      size_t total;
      while (total = fread(buf, sizeof(char), 10, fp))
        SDLNet_TCP_Send(client, buf, total);
      fclose(fp);
    }
    else {
      const char* msg = "<html><body>Pagina nao encontrada.</body></html>";
      SDLNet_TCP_Send(client, msg, strlen(msg) + 1);
    }
    
    SDLNet_TCP_Close(client);
  }
  
  SDLNet_TCP_Close(tcpsock);
  
  close();
  
  return 0;
}
