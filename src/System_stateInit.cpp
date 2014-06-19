/*
 * System_stateInit.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <vector>

// local
#include "Network.hpp"

using namespace std;
using namespace network;

void System::stateInit() {
  Address addr;
  vector<char> data = mainUDPSocket.recv(addr);
  if (data.size()) {
    //TCPConnection x(Address(addr.ip, ));
  }
}
