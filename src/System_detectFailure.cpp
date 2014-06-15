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
  for (auto it = peers.begin(); it != peers.end();) {
    if (it->second.time() >= MS_DETECTFAILURE) {
      printf("\t%x caiu\n", it->first);
      fflush(stdout);
      peers.erase(it++);
    }
    else
      ++it;
  }
}
