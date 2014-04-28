/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_net.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>

using namespace std;

enum KeyState {
  PRESSED,
  JUST_PRESSED,
  RELEASED,
  JUST_RELEASED
};

KeyState keys[300];
bool peerOn = false;
SDL_Thread* peerThread = nullptr;

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


void input() {
  for (auto& i : keys) {
    if (i == JUST_PRESSED)
      i = PRESSED;
    else if (i == JUST_RELEASED)
      i = RELEASED;
  }
  
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        keys[event.key.keysym.sym] = JUST_PRESSED;
        break;
      case SDL_KEYUP:
        keys[event.key.keysym.sym] = JUST_RELEASED;
        break;
      default:
        break;
    }
  }
}

int peerListen(void*) {
  map<Uint32, Uint32> peers;
  
  UDPsocket listenSocket = SDLNet_UDP_Open(6969);
  UDPpacket* packet = SDLNet_AllocPacket(1);
  while (peerOn) {
    if (SDLNet_UDP_Recv(listenSocket, packet)) {
      peers[packet->address.host] = SDL_GetTicks();
      printAddr(packet->address.host, packet->address.port);
    }
    for (auto it = peers.begin(); it != peers.end();) {
      if (SDL_GetTicks() - it->second  >= 10000) {
        printf("\t%x caiu\n", it->first);
        peers.erase(it++);
      }
      else
        ++it;
    }
  }
  SDLNet_FreePacket(packet);
  SDLNet_UDP_Close(listenSocket);
  return 0;
}

int peerSpeak(void*) {
  UDPsocket speakSocket = SDLNet_UDP_Open(0);
  IPaddress discoverAddr;
  SDLNet_ResolveHost(&discoverAddr, "255.255.255.255", 6969);
  UDPpacket* packet = SDLNet_AllocPacket(1);
  packet->address.host = discoverAddr.host;
  packet->address.port = discoverAddr.port;
  packet->len = 1;
  *((Uint8*)packet->data) = (Uint8)0xFF;
  while (peerOn) {
    SDLNet_UDP_Send(speakSocket, -1, packet);
    SDL_Delay(5000);
  }
  SDLNet_FreePacket(packet);
  SDLNet_UDP_Close(speakSocket);
  return 0;
}

int peer(void*) {
  SDL_Thread* speakThread = SDL_CreateThread(peerSpeak, "peerSpeak", nullptr);
  SDL_Thread* listenThread = SDL_CreateThread(peerListen, "peerListen", nullptr);
  SDL_WaitThread(speakThread, nullptr);
  SDL_WaitThread(listenThread, nullptr);
  return 0;
}

void startPeer() {
  peerThread = SDL_CreateThread(peer, "peer", nullptr);
}

void stopPeer() {
  SDL_WaitThread(peerThread, nullptr);
}

int main(int argc, char* argv[]) {
  init();
  
  SDL_Window* window = SDL_CreateWindow("final-torrent", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 480, 0);
  
  
  while (!SDL_QuitRequested()) {
    input();
    
    if (keys[SDLK_a] == JUST_PRESSED) {
      peerOn = !peerOn;
      if (peerOn)
        startPeer();
      else
        stopPeer();
      printf("peer status: %d\n", (int)peerOn);
    }
    
    
    //fflush(stdout);
    
    SDL_Delay(50);
  }
  
  SDL_DestroyWindow(window);
  
  close();
  return 0;
}
