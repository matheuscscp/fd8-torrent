/*
 * System_stateIdle.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

void System::stateIdle() {
  speak();
  answerRequests();
  httpServer();
}
