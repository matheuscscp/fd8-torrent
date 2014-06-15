/*
 * Concurrency.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "Concurrency.hpp"

// lib
#include <SDL.h>

using namespace std;

namespace concurrency {

// =============================================================================
// class Thread;
// =============================================================================

Thread::Thread(const function<void()>& f) : f(f), started(false), thread(nullptr) {
  
}

void Thread::start() {
  // check if already started
  if (started)
    return;
  
  // start
  started = true;
  thread = SDL_CreateThread(exec, nullptr, new function<void()>(f));
}

void Thread::join() {
  // check if thread exists
  if (!thread)
    return;
  
  // join
  SDL_WaitThread((SDL_Thread*)thread, nullptr);
  thread = nullptr;
}

void Thread::sleep(uint32_t ms, const bool* keepCondition) {
  // for naps, or if there is no wakeup condition
  if (ms <= 50 || keepCondition == nullptr) {
    SDL_Delay(ms);
    return;
  }
  
  uint32_t now = SDL_GetTicks();
  uint32_t time = now + ms;
  int cont = 0;
  do {
    ms = time - now;
    SDL_Delay(ms < 50 ? ms : 50);
    now = SDL_GetTicks();
    cont++;
  } while (true == *keepCondition && now < time);
}

int Thread::exec(void* func) {
  auto fptr = (function<void()>*)func;
  (*fptr)();
  delete fptr;
  return 0;
}

// =============================================================================
// class Mutex;
// =============================================================================

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

// =============================================================================
// class AtomicBase;
// =============================================================================

AtomicBase::~AtomicBase() {
  
}

} // namespace concurrency
