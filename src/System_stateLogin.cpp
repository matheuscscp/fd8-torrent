/*
 * System_stateLogin.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "FileSystem.hpp"
#include "Defines.hpp"

using namespace concurrency;

void System::changeToLogin() {
  users.erase(localAddress.ip);
  loginSyncTimer.start();
  httpThread = Thread([this]() {
    while (state == newState) {
      loginHttpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
}

void System::stateLogin() {
  listen();
  detectFailure();
}
