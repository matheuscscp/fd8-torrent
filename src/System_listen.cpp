/*
 * System_listen.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <cstdio>
#include <vector>

using namespace std;
using namespace network;

static void printAddr(uint32_t host, uint16_t port) {
  printf("\tAddress: %d", ((uint8_t*)&host)[0]);
  for (int i = 1; i < 4; i++)
    printf(".%d", ((uint8_t*)&host)[i]);
  printf(" %d\n", Address::ntohs(port));
}

void System::listen() {
  vector<char> data;
  Address addr;
  
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
