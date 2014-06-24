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

using namespace concurrency;

void System::changeToIdle() {
  state = STATE_IDLE;
  httpThread = Thread([this]() {
    while (state == STATE_IDLE) {
      httpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
}

void System::stateIdle() {
  listen();
  speak();
  detectFailure();
  executeProtocol();
}
