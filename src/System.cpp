/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Thread.hpp"
#include "Defines.hpp"

using namespace std;
using namespace network;

bool System::started = false;
bool System::initialized = false;

bool System::start() {
  if (started | initialized)
    return false;
  started = true;
  Thread([]() {
    {
      System sys;
      initialized = true;
      sys.run();
    }
    initialized = false;
  }).start();
  return true;
}

bool System::stop() {
  if (!started | !initialized)
    return false;
  started = false;
  return true;
}

bool System::changing() {
  return started ^ initialized;
}

bool System::running() {
  return started & initialized;
}

System::System() :
localAddress(Address::local()),
mainUDPSocket(
  Address(localAddress.ip, Address::htons(UDP_LISTEN)),
  Address(IP_LISTEN).ip
) {
  // httpServer
  {
//    TCPsocket* httpServer = new TCPsocket;
//    IPaddress addr;
//    SDLNet_ResolveHost(&addr, nullptr, TCP_WEBSERVER);
//    *httpServer = SDLNet_TCP_Open(&addr);
//    globals["httpServer"] = new Atomic<TCPsocket>(httpServer);
  }
}

void System::run() {
  while (started) {
    speak();
    listen();
    detectFailure();
    httpServer();
    Thread::sleep(MS_SLEEP);
  }
}
