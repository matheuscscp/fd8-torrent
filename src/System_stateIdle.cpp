/*
 * System_stateIdle.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "FileSystem.hpp"

using namespace concurrency;

void System::changeToIdle() {
  FileSystem::init(localAddress.ip);
  requestSystemState();
  httpThread = Thread([this]() {
    while (state == newState) {
      httpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
}

void System::stateIdle() {
  executeProtocol();
  speak();
  listen();
  detectFailure();
}
