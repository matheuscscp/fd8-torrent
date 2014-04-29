/*
 * Globals.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

// lib
#include <SDL_stdinc.h>

// local
#include "Globals.hpp"

using namespace std;

map<string, LockableBase*> Globals::globals;
bool Globals::isInit = false;

void Globals::init() {
  // check if is already initialized
  if (isInit)
    return;
  isInit = true;
  
  // systemOn
  bool* systemOn = new bool;
  *systemOn = true;
  globals["systemOn"] = new Lockable<bool>(systemOn);
  
  // peers
  globals["peers"] = new Lockable<map<Uint32, Uint32>>(new map<Uint32, Uint32>);
}

void Globals::close() {
  // check if is already closed (or still not initialized)
  if (!isInit)
    return;
  isInit = false;
  
  for (auto& kv : globals)
    delete kv.second;
}
