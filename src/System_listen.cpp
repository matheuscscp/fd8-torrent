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
using namespace helpers;

void System::listen() {
  ByteQueue data;
  Address addr;
  
  data = mainUDPSocket.recv(addr);
  //if (!data.size() || addr.ip == localAddress.ip)
  if (!data.size())
    return;
  
  // update timeout
  User& user = users[addr.ip];
  user.name = data.pop<string>();
  user.timer.start();
  
  // print
  printf("Beacon: %s - User: %s\n", addr.toString().c_str(), user.name.c_str());
  fflush(stdout);
}
