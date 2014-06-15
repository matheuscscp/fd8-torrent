/*
 * SystemDetectFailure.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "SystemDetectFailure.hpp"

// standard
#include <cstdio>
#include <map>

// lib
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>

// local
#include "Globals.hpp"
#include "Thread.hpp"
#include "Helpers.hpp"
#include "Defines.hpp"

using namespace std;
using namespace helpers;

void SystemDetectFailure() {
  static map<uint32_t, Timer>& peers = Globals::get<map<uint32_t, Timer>>("peers").value();
  
  for (auto it = peers.begin(); it != peers.end();) {
    if (it->second.time() >= MS_DETECTFAILURE) {
      printf("\t%x caiu\n", it->first);
      fflush(stdout);
      peers.erase(it++);
    }
    else
      ++it;
  }
}
