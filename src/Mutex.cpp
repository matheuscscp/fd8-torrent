/*
 * Mutex.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

// lib
#include <SDL.h>

// local
#include "Mutex.hpp"

Mutex::Mutex() {
  mutex = SDL_CreateMutex();
}

Mutex::~Mutex() {
  SDL_DestroyMutex((SDL_mutex*)mutex);
}

void Mutex::lock() {
  SDL_mutexP((SDL_mutex*)mutex);
}

void Mutex::unlock() {
  SDL_mutexV((SDL_mutex*)mutex);
}
