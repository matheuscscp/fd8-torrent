/*
 * Globals.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

// this
#include "Globals.hpp"

// local
#include "Platform.hpp"

using namespace std;
using namespace platform;

map<string, AtomicBase*> Globals::globals;
bool Globals::isInit = false;

#include <cstdio>

void Globals::init() {
  // check if is already initialized
  if (isInit)
    return;
  isInit = true;
  
  // systemOn
  {
    bool* systemOn = new bool;
    *systemOn = true;
    globals["systemOn"] = new Atomic<bool>(systemOn);
  }
  
  // peers
  {
    globals["peers"] = new Atomic<map<uint32_t, uint32_t>>(new map<uint32_t, uint32_t>);
  }
  
  // localIP
  {
    uint32_t* ip = new uint32_t;
    *ip = getLocalIP();
    globals["localIP"] = new Atomic<uint32_t>(ip);
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
