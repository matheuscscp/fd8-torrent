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
#include <SDL.h>

// local
#include "Defines.hpp"

using namespace std;

namespace helpers {

// =============================================================================
// class StaticInitializer;
// =============================================================================

StaticInitializer::StaticInitializer(function<void()> f) {
  f();
}

// =============================================================================
// class Timer;
// =============================================================================

Timer::Timer() : started(false), paused(false), initialTime(0), pauseTime(0) {
  
}

void Timer::start() {
  started = true;
  paused = false;
  initialTime = SDL_GetTicks();
}

void Timer::pause() {
  if (!paused) {
    paused = true;
    pauseTime = SDL_GetTicks();
  }
}

void Timer::resume() {
  if (paused) {
    paused = false;
    initialTime += SDL_GetTicks() - pauseTime;
  }
}

void Timer::reset() {
  started = false;
}

uint32_t Timer::time() {
  if (!started)
    return 0;
  if (paused)
    return pauseTime - initialTime;
  return SDL_GetTicks() - initialTime;
}

bool Timer::counting() {
  return started;
}

// =============================================================================
// functions
// =============================================================================

void openBrowser() {
  char cmd[100];
#ifdef _WIN32
  sprintf(cmd, "start http://localhost:%s", TCP_HTTPSERVER);
#else
  sprintf(cmd, "sensible-browser http://localhost:%s", TCP_HTTPSERVER);
#endif
  system(cmd);
}

} // namespace helpers
