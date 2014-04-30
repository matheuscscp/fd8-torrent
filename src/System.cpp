/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <cstdio>

// lib
#include <SDL_net.h>
#include <SDL_stdinc.h>

// local
#include "Globals.hpp"
#include "Lockable.hpp"
#include "SystemDetectFailure.hpp"
#include "SystemListen.hpp"
#include "SystemSpeak.hpp"
#include "Define.hpp"

Thread<System::run>* System::thread = nullptr;
bool System::running = false;

bool System::start() {
  if (running)
    return false;
  running = true;
  thread = new Thread<run>;
  thread->start();
  return true;
}

bool System::stop() {
  if (!running)
    return false;
  running = false;
  Globals::get<bool>("systemOn").value() = false;
  thread->join();
  return true;
}

bool System::isRunning() {
  return running;
}

static void printAddr(Uint32 host) {
  printf("%d", ((Uint8*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((Uint8*)&host)[i]);
  printf("\n");
}

void System::run() {
  // init
  Globals::init();
  
  // resolve ip
  {
    UDPsocket sock = SDLNet_UDP_Open(0);
    UDPsocket socklist = SDLNet_UDP_Open(FD8_UDP_PORT_RESOLVE_IP);
    UDPpacket* pack = SDLNet_AllocPacket(1);
    IPaddress ip;
    SDLNet_ResolveHost(&ip, "255.255.255.255", FD8_UDP_PORT_RESOLVE_IP);
    pack->address.host = ip.host;
    pack->address.port = ip.port;
    pack->len = 1;
    *((Uint8*)pack->data) = (Uint8)0xFF;
    SDLNet_UDP_Send(sock, -1, pack);
    SDLNet_UDP_Recv(socklist, pack);
    printAddr(pack->address.host);
    SDLNet_FreePacket(pack);
  }
  
  // all system threads
  Thread<SystemSpeak> speak;
  Thread<SystemListen> listen;
  Thread<SystemDetectFailure> detectFailure;
  
  // start all threads
  speak.start();
  listen.start();
  detectFailure.start();
  
  // join all threads
  speak.join();
  listen.join();
  detectFailure.join();
  
  // close
  Globals::close();
}
