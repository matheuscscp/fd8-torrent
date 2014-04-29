/*
 * Thread.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// lib
#include <SDL.h>

// local
#include "Thread.hpp"

void Thread_sleep(Uint32 ms, const bool* keepCondition) {
  // for naps, or if there is no wakeup condition
  if (ms <= 50 || keepCondition == nullptr) {
    SDL_Delay(ms);
    return;
  }
  
  Uint32 now = SDL_GetTicks();
  Uint32 time = now + ms;
  int cont = 0;
  do {
    ms = time - now;
    SDL_Delay(ms < 50 ? ms : 50);
    now = SDL_GetTicks();
    cont++;
  } while (true == *keepCondition && now < time);
}

void Thread_sleep(Uint32 ms) {
  Thread_sleep(ms, nullptr);
}
