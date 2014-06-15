/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"

using namespace std;
using namespace concurrency;
using namespace network;

Thread* System::thread = nullptr;
bool System::started = false;
bool System::initialized = false;

bool System::start() {
  if (started | initialized)
    return false;
  started = true;
  thread = new Thread([]() {
    {
      System sys;
      initialized = true;
      sys.run();
    }
    initialized = false;
  });
  thread->start();
  return true;
}

bool System::stop(bool wait) {
  if (!started | !initialized)
    return false;
  started = false;
  if (wait)
    thread->join();
  delete thread;
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
multicastAddress(IP_MULTICAST, UDP_MULTICAST),
mainUDPSocket(multicastAddress),
httpTCPServer(TCP_HTTPSERVER)
{
  
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
