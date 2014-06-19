/*
 * System_detectFailure.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <cstdio>

// local
#include "Defines.hpp"

void System::detectFailure() {
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE) {
      printf("\t%x caiu\n", it->first);
      fflush(stdout);
      users.erase(it++);
    }
    else
      ++it;
  }
}
