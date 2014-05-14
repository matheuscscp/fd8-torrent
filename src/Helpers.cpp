/*
 * Helpers.cpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

// this
#include "Helpers.hpp"

// standard
#include <cstdio>

// lib
#include <SDL_net.h>

using namespace std;

namespace helpers {

uint32_t str2Network(const char* str) {
  IPaddress addr;
  SDLNet_ResolveHost(&addr, str, 0);
  return addr.host;
}

string network2str(uint32_t ipaddr) {
  char tmp[16];
  sprintf(tmp, "%d.%d.%d.%d",
    int(((uint8_t*)&ipaddr)[0]),
    int(((uint8_t*)&ipaddr)[1]),
    int(((uint8_t*)&ipaddr)[2]),
    int(((uint8_t*)&ipaddr)[3])
  );
  return string(tmp);
}

} // namespace helpers
