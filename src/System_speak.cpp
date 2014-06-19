/*
 * System_speak.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <vector>

// local
#include "Defines.hpp"

using namespace std;
using namespace network;
using namespace helpers;

void System::speak() {
  static Timer timer;
  static const char data = 0xFF;
  
  if (!timer.counting())
    timer.start();
  if (timer.time() > MS_SPEAK) {
    mainUDPSocket.send(multicastAddress, &data, 1);
    timer.start();
  }
}
