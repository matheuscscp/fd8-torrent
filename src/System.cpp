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
#include "FileSystem.hpp"

using namespace std;
using namespace concurrency;
using namespace network;

Thread* System::thread = nullptr;
bool System::started = false;
bool System::initialized = false;

System::User::User() {
  
}

System::User::User(const string& name) : name(name) {
  
}

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
state(STATE_NONE),
newState(STATE_LOGIN),
localAddress(Address::local()),
broadcastAddress("255.255.255.255", TCPUDP_MAIN),
multicastAddress(IP_MAIN, TCPUDP_MAIN),
mainUDPSocket(multicastAddress, SIZE_MULTICAST_MAXLEN),
mainTCPServer(TCPUDP_MAIN),
httpTCPServer(TCP_HTTPSERVER),
httpThread([]() {}),
downloadsRemaining(0)
{
  
}

System::~System() {
  state = STATE_NONE;
  httpThread.join();
  while (downloadsRemaining)
    Thread::sleep(MS_SLEEP);
}

void System::run() {
  while (started) {
    switch (state) {
      case STATE_LOGIN: stateLogin(); break;
      case STATE_IDLE:  stateIdle();  break;
      default:                        break;
    }
    if (newState != state)
      change();
    Thread::sleep(MS_SLEEP);
  }
}

void System::change() {
  httpThread.join();
  state = newState;
  switch (newState) {
    case STATE_LOGIN: changeToLogin();  break;
    case STATE_IDLE:  changeToIdle();   break;
    default:                            break;
  }
}
