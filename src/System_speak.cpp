/*
 * System_speak.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"

using namespace std;
using namespace network;
using namespace helpers;

void System::speak() {
  static Timer timer;
  
  if (!timer.counting())
    timer.start();
  if (timer.time() > MS_SPEAK) {
    mainUDPSocket.send(broadcastAddress, ByteQueue().push(users[localAddress.ip].name));
    mainUDPSocket.send(multicastAddress, ByteQueue().push(users[localAddress.ip].name));
    timer.start();
  }
}
