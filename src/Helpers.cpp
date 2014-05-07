/*
 * Helpers.cpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

// this
#include "Helpers.hpp"

// lib
#include <SDL_net.h>

namespace helpers {

uint32_t str2Network(const char* str) {
  IPaddress addr;
  SDLNet_ResolveHost(&addr, str, 0);
  return addr.host;
}

} // namespace helpers
