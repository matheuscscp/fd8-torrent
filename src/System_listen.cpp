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

void System::listen() {
  vector<char> data;
  Address addr;
  
  data = mainUDPSocket.recv(addr);
  if (!data.size())
    return;
  if (addr.ip == localAddress.ip)
    return;
  
  // update timeout
  users[addr.ip].timer.start();
  
  // print
  printf("Total bytes: %d", data.size());
  printf("\tAddress: %s\n", addr.toString().c_str());
  for (auto byte : data)
    printf("%x ", ((int)byte) & 0xFF);
  printf("\n");
  fflush(stdout);
}
