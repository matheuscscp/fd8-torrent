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
#include <cstdio>

// lib
#include <SDL2/SDL_net.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Define.hpp"
#include "Platform.hpp"

using namespace std;
using namespace platform;

static void printAddr(Uint32 host, Uint16 port) {
  printf("\tAddress: %d", ((Uint8*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((Uint8*)&host)[i]);
  printf(" %d\n", SDLNet_Read16(&port));
}

void SystemListen() {
  MulticastSocket sock(Globals::get<Uint32>("localIP").value(), UDP_LISTEN_HOST, IP_MULTICAST_NET);
  bool& systemOn = Globals::get<bool>("systemOn").value();
  Lockable<map<Uint32, Uint32>>& peers = Globals::get<map<Uint32, Uint32>>("peers");
  IPaddress addr;
  vector<char> pack;
  while (systemOn) {
    pack = sock.read(addr.host, addr.port);
    if (pack.size()) {
      peers.lock();
      peers.value()[addr.host] = SDL_GetTicks();
      peers.unlock();
      printf("Total bytes: %d", pack.size());
      printAddr(addr.host, addr.port);
      for (auto byte : pack)
        printf("%x ", ((int)byte) & 0xFF);
      printf("\n");
      fflush(stdout);
    }
    Thread::sleep(50);
  }
}
