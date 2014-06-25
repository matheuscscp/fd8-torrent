/*
 * System_detectFailure.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"

void System::detectFailure() {
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE)
      users.erase(it++);
    else
      ++it;
  }
}
