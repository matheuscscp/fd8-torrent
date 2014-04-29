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

using namespace std;

void SystemDetectFailure() {
  bool& systemOn = Globals::get<bool>("systemOn").value();
  Lockable<map<Uint32, Uint32>>& peers = Globals::get<map<Uint32, Uint32>>("peers");
  while (systemOn) {
    peers.lock();
    for (auto it = peers.value().begin(); it != peers.value().end();) {
      if (SDL_GetTicks() - it->second  >= 10000) {
        printf("\t%x caiu\n", it->first);
        peers.value().erase(it++);
      }
      else
        ++it;
    }
    peers.unlock();
    Thread_sleep(50);
  }
}
