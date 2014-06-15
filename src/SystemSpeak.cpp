/*
 * SystemSpeak.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "SystemSpeak.hpp"

// standard
#include <vector>

// lib
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_stdinc.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Defines.hpp"
#include "Network.hpp"
#include "Helpers.hpp"

using namespace std;
using namespace helpers;
using namespace network;

void SystemSpeak() {
  static Timer timer;
  static UDPSocket& mainUDPSocket = Globals::get<UDPSocket>("mainUDPSocket").value();
  static Address multicastAddr(IP_LISTEN, UDP_LISTEN);
  static vector<char> data;
  static StaticInitializer staticInitializer([&]() {
    data.push_back(0xFF);
  });
  
  if (timer.time() > MS_SPEAK) {
    mainUDPSocket.send(multicastAddr, data);
    timer.start();
  }
}
