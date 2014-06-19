/*
 * System_stateInit.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

#include "System.hpp"
#include "Network.hpp"

using namespace network;

void System::stateInit() {
  Address addr;
  mainUDPSocket.recv(addr);
}
