/*
 * multicast.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_main.h>
#include <SDL_net.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>

using namespace std;

static void init() {
  if (SDL_Init(0)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    exit(0);
  }
  
  if (SDLNet_Init()) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(0);
  }
}

static void close() {
  SDLNet_Quit();
  SDL_Quit();
}

static void printAddr(Uint32 host, Uint16 port) {
  printf("\tAddress: %d", ((Uint8*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((Uint8*)&host)[i]);
  printf(" %d\n", SDLNet_Read16(&port));
}



int multicast(int argc, char* argv[]) {
  init();
  
  int port;
  UDPpacket* packet = SDLNet_AllocPacket(512);
  sscanf(argv[2], "%d", &port);
  if (string(argv[1]) == "-s") { //server
    UDPsocket server = SDLNet_UDP_Open(port);
    while (true) {
      SDL_Delay(50);
      if (SDLNet_UDP_Recv(server, packet)) {
        printf("UDP Packet incoming\n");
        printf("\tChan:    %d\n", packet->channel);
        printf("\tData:    %s\n", (char *)packet->data);
        printf("\tLen:     %d\n", packet->len);
        printf("\tMaxlen:  %d\n", packet->maxlen);
        printf("\tStatus:  %d\n", packet->status);
        printAddr(packet->address.host, packet->address.port);
        
        /* Quit if packet contains "quit" */
        if (string((char *)packet->data) == "quit")
          break;
      }
    }
  }
  else { //client
    UDPsocket client = SDLNet_UDP_Open(0);
    IPaddress svaddr;
    SDLNet_ResolveHost(&svaddr, argv[1], port);
    packet->address.host = svaddr.host;
    packet->address.port = svaddr.port;
    while (true) {
      SDL_Delay(50);
      printf("Fill the buffer: ");
      scanf("%s", (char*)packet->data);
      packet->len = strlen((char*)packet->data) + 1;
      SDLNet_UDP_Send(client, -1, packet);
      if (string((char *)packet->data) == "quit")
        break;
    }
  }
  SDLNet_FreePacket(packet);
  
  close();
  
  return 0;
}
