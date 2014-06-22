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
// class ByteQueue;
// =============================================================================

ByteQueue::ByteQueue(size_t size) {
  buf.resize(size);
}

size_t ByteQueue::size() const {
  return buf.size();
}

void ByteQueue::resize(size_t size) {
  buf.resize(size);
}

void* ByteQueue::ptr() const {
  return buf.size() ? (void*)&buf[0] : nullptr;
}

ByteQueue& ByteQueue::push(const void* data, size_t maxlen) {
  buf.insert(buf.end(), (uint8_t*)data, ((uint8_t*)data) + maxlen);
  return *this;
}

ByteQueue& ByteQueue::push(const string& data, bool withoutNullTermination) {
  buf.insert(buf.end(), (uint8_t*)data.c_str(), ((uint8_t*)data.c_str()) + data.size());
  if (!withoutNullTermination)
    buf.push_back(uint8_t(0));
  return *this;
}

size_t ByteQueue::pop(void* data, size_t maxlen) {
  size_t total = maxlen <= buf.size() ? maxlen : buf.size();
  if (!total)
    return 0;
  memcpy(data, (const void*)&buf[0], total);
  buf.erase(buf.begin(), buf.begin() + total);
  return total;
}

string ByteQueue::pop(size_t maxlen) {
  string data(maxlen <= buf.size() ? maxlen : buf.size(), '0');
  if (data.size()) {
    memcpy((void*)data.c_str(), (const void*)&buf[0], data.size());
    buf.erase(buf.begin(), buf.begin() + data.size());
  }
  return data;
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

list<string> explode(const string& str, char delim) {
  list<string> result;
  string tmp;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] != delim)
      tmp += str[i];
    else if (tmp.size()) {
      result.push_back(tmp);
      tmp = "";
    }
  }
  if (tmp.size())
    result.push_back(tmp);
  return result;
}

pair<string, string> extractFirst(const string& str, char delim) {
  pair<string, string> result;
  int i;
  for (i = 0; i < int(str.size()) && str[i] == delim; i++);
  for (; i < int(str.size()) && str[i] != delim; i++);
  if (i < int(str.size())) {
    result.first = str.substr(0, i);
    result.second = str.substr(i, str.size());
  }
  else
    result.first = str;
  return result;
}

pair<string, string> extractLast(const string& str, char delim) {
  pair<string, string> result;
  int i;
  for (i = int(str.size()) - 1; i >= 0 && str[i] != delim; i--);
  for (; i >= 0 && str[i] == delim; i--);
  if (i >= 0) {
    result.first = str.substr(0, i + 1);
    result.second = str.substr(i + 1, str.size());
  }
  else
    result.first = str;
  return result;
}

} // namespace helpers
