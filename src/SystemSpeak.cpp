/*
 * SystemSpeak.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "SystemSpeak.hpp"

// lib
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_stdinc.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"

void SystemSpeak() {
  UDPsocket speakSocket = SDLNet_UDP_Open(0);
  IPaddress discoverAddr;
  SDLNet_ResolveHost(&discoverAddr, "255.255.255.255", 6969);
  UDPpacket* packet = SDLNet_AllocPacket(1);
  packet->address.host = discoverAddr.host;
  packet->address.port = discoverAddr.port;
  packet->len = 1;
  *((Uint8*)packet->data) = (Uint8)0xFF;
  bool& systemOn = Globals::get<bool>("systemOn").value();
  while (systemOn) {
    SDLNet_UDP_Send(speakSocket, -1, packet);
    Thread_sleep(5000, &systemOn);
  }
  SDLNet_FreePacket(packet);
  SDLNet_UDP_Close(speakSocket);
}
