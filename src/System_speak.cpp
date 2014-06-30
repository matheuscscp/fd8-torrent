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
    User& user = users[localAddress.ip];
    ByteQueue data;
    data.push(user.sessionID).push(user.name);
    mainUDPSocket.send(broadcastAddress, data);
    mainUDPSocket.send(multicastAddress, data);
    timer.start();
  }
}
