/*
 * Globals.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

// this
#include "Globals.hpp"

// standard
#include <set>

// lib
#include <SDL_net.h>

// local
#include "Network.hpp"
#include "Defines.hpp"
#include "Helpers.hpp"

using namespace std;
using namespace helpers;
using namespace network;

map<string, AtomicBase*> Globals::globals;
bool Globals::isInit = false;

void Globals::init() {
  // check if is already initialized
  if (isInit)
    return;
  
  // systemOn
  {
    bool* systemOn = new bool;
    *systemOn = true;
    globals["systemOn"] = new Atomic<bool>(systemOn);
  }
  
  // peers
  {
    globals["peers"] = new Atomic<map<uint32_t, Timer>>(new map<uint32_t, Timer>);
  }
  
  // localIP
  {
    uint32_t* localIP = new uint32_t;
    *localIP = Address::local().ip;
    globals["localIP"] = new Atomic<uint32_t>(localIP);
  }
  
  // users
  {
    globals["users"] = new Atomic<set<string>>(new set<string>);
  }
  
  // mainUDPSocket
  {
    Address tmp(IP_LISTEN, UDP_LISTEN);
    globals["mainUDPSocket"] = new Atomic<UDPSocket>(new UDPSocket(
      Address(Globals::get<uint32_t>("localIP").value(), tmp.port), tmp.ip
    ));
  }
  
  // httpServer
  {
    TCPsocket* httpServer = new TCPsocket;
    IPaddress addr;
    SDLNet_ResolveHost(&addr, nullptr, TCP_WEBSERVER);
    *httpServer = SDLNet_TCP_Open(&addr);
    globals["httpServer"] = new Atomic<TCPsocket>(httpServer);
  }
  
  isInit = true;
}

void Globals::close() {
  // check if is already closed (or still not initialized)
  if (!isInit)
    return;
  
  for (auto& kv : globals)
    delete kv.second;
  
  isInit = false;
}

bool Globals::ready() {
  return isInit;
}
