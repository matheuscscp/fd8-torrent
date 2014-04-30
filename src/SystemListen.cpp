/*
 * SystemListen.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "SystemListen.hpp"

// standard
#include <map>

// lib
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Define.hpp"

using namespace std;

static void printAddr(Uint32 host, Uint16 port) {
  printf("\tAddress: %d", ((Uint8*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((Uint8*)&host)[i]);
  printf(" %d\n", SDLNet_Read16(&port));
}

void SystemListen() {
  UDPsocket listenSocket = SDLNet_UDP_Open(FD8_UDP_PORT_LISTEN);
  UDPpacket* packet = SDLNet_AllocPacket(1);
  bool& systemOn = Globals::get<bool>("systemOn").value();
  Lockable<map<Uint32, Uint32>>& peers = Globals::get<map<Uint32, Uint32>>("peers");
  while (systemOn) {
    if (SDLNet_UDP_Recv(listenSocket, packet)) {
      peers.lock();
      peers.value()[packet->address.host] = SDL_GetTicks();
      peers.unlock();
      printAddr(packet->address.host, packet->address.port);
    }
    Thread_sleep(50);
  }
  SDLNet_FreePacket(packet);
  SDLNet_UDP_Close(listenSocket);
}
