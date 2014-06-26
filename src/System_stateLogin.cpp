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

void System::changeToLogin() {
  users.clear();
  FileSystem::init(localAddress.ip);
  state = STATE_LOGIN;
}

void System::stateLogin() {
  requestSystemState();
  loginHttpServer();
}
