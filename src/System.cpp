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
    network::init();
    {
      System sys;
      initialized = true;
      sys.run();
    }
    network::close();
    initialized = false;
  });
  thread->start();
  return true;
}

void System::stop(bool wait) {
  if (wait && thread == nullptr) {
    while (initialized)
      Thread::sleep(200);
    return;
  }
  if (!started | !initialized)
    return;
  started = false;
  if (wait)
    thread->join();
  delete thread;
  thread = nullptr;
  return;
}

bool System::changing() {
  return started ^ initialized;
}

bool System::running() {
  return started & initialized;
}

System::System() :
state(STATE_INIT),
localAddress(Address::local()),
multicastAddress(IP_MAIN, TCPUDP_MAIN),
mainUDPSocket(multicastAddress, SIZE_MULTICAST_MAXLEN),
mainTCPServer(TCPUDP_MAIN),
httpTCPServer(TCP_HTTPSERVER),
httpThread([]() {})
{
  initTimer.start();
}

void System::run() {
  while (started) {
    switch (state) {
      case STATE_INIT:  stateInit();  break;
      case STATE_LOGIN: stateLogin(); break;
      case STATE_IDLE:  stateIdle();  break;
    }
    Thread::sleep(MS_SLEEP);
  }
}
