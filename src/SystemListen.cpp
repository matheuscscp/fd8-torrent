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
#include "Defines.hpp"
#include "Network.hpp"
#include "Helpers.hpp"

using namespace std;
using namespace network;
using namespace helpers;

static void printAddr(uint32_t host, uint16_t port) {
  printf("\tAddress: %d", ((uint8_t*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((uint8_t*)&host)[i]);
  printf(" %d\n", Address::ntohs(port));
}

void SystemListen() {
  vector<char> data;
  static UDPSocket& mainUDPSocket = Globals::get<UDPSocket>("mainUDPSocket").value();
  Address addr;
  static map<uint32_t, Timer>& peers = Globals::get<map<uint32_t, Timer>>("peers").value();
  
  data = mainUDPSocket.recv(addr);
  if (!data.size())
    return;
  
  // update timeout
  peers[addr.ip].start();
  
  // print
  printf("Total bytes: %d", data.size());
  printAddr(addr.ip, addr.port);
  for (auto byte : data)
    printf("%x ", ((int)byte) & 0xFF);
  printf("\n");
  fflush(stdout);
}
