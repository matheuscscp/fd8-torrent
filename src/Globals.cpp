/*
 * Globals.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

// this
#include "Globals.hpp"

// lib
#include <SDL_net.h>

using namespace std;

map<string, LockableBase*> Globals::globals;
bool Globals::isInit = false;

void Globals::init() {
  // check if is already initialized
  if (isInit)
    return;
  isInit = true;
  
  // systemOn
  {
    bool* systemOn = new bool;
    *systemOn = true;
    globals["systemOn"] = new Lockable<bool>(systemOn);
  }
  
  // peers
  {
    globals["peers"] = new Lockable<map<Uint32, Uint32>>(new map<Uint32, Uint32>);
  }
  
  // localIP
  {
    Uint32* localIP = new Uint32;
    IPaddress addr;
    SDLNet_ResolveHost(&addr, nullptr, 0);
    SDLNet_ResolveHost(&addr, SDLNet_ResolveIP(&addr), 0);
    *localIP = addr.host;
    globals["localIP"] = new Lockable<Uint32>(localIP);
  }
}

void Globals::close() {
  // check if is already closed (or still not initialized)
  if (!isInit)
    return;
  isInit = false;
  
  for (auto& kv : globals)
    delete kv.second;
}
