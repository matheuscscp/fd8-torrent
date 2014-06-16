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

struct ThreadInfo {
  function<void()> f;
  bool* terminated;
  ThreadInfo(const function<void()>& f, bool* terminated) :
  f(f), terminated(terminated) {}
  ~ThreadInfo() { delete terminated; }
};

Thread::Thread(const function<void()>& f) :
f(f), thread(nullptr), joined(false), terminated(new bool) {
  *terminated = false;
}

Thread::~Thread() {
  if (!thread)
    delete terminated;
  else if (!joined)
    SDL_DetachThread((SDL_Thread*)thread);
}

Thread::Thread(const Thread& other) :
f(other.f),
thread(other.thread),
joined(other.joined),
terminated(other.terminated)
{
  auto tmp = (Thread&)other;
  
  tmp.joined = true;
  tmp.terminated = new bool;
  *tmp.terminated = true;
}

Thread& Thread::operator=(const Thread& other) {
  f = other.f;
  thread = other.thread;
  joined = other.joined;
  terminated = other.terminated;
  
  auto tmp = (Thread&)other;
  
  tmp.joined = true;
  tmp.terminated = new bool;
  *tmp.terminated = true;
  
  return *this;
}

Thread::Thread(Thread&& other) :
f(other.f),
thread(other.thread),
joined(other.joined),
terminated(other.terminated)
{
  auto tmp = (Thread&)other;
  
  tmp.joined = true;
  tmp.terminated = new bool;
  *tmp.terminated = true;
}

Thread& Thread::operator=(Thread&& other) {
  f = other.f;
  thread = other.thread;
  joined = other.joined;
  terminated = other.terminated;
  
  auto tmp = (Thread&)other;
  
  tmp.joined = true;
  tmp.terminated = new bool;
  *tmp.terminated = true;
  
  return *this;
}

void Thread::start() {
  if (thread || joined)
    return;
  thread = SDL_CreateThread(exec, nullptr, new ThreadInfo(f, terminated));
}

void Thread::join() {
  if (joined)
    return;
  SDL_WaitThread((SDL_Thread*)thread, nullptr);
  joined = true;
}

bool Thread::running() {
  return thread && !terminated;
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

int Thread::exec(void* threadInfo) {
  auto info = (ThreadInfo*)threadInfo;
  info->f();
  *info->terminated = true;
  delete info;
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
