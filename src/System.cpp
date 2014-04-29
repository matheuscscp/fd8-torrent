/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Globals.hpp"
#include "SystemListen.hpp"
#include "SystemSpeak.hpp"
#include "SystemDetectFailure.hpp"
#include "Thread.hpp"

Thread<System::run>* System::thread = nullptr;
bool System::running = false;

bool System::start() {
  if (running)
    return false;
  running = true;
  thread = new Thread<run>;
  thread->start();
  return true;
}

bool System::stop() {
  if (!running)
    return false;
  running = false;
  Globals::get<bool>("systemOn").value() = false;
  thread->join();
  return true;
}

bool System::isRunning() {
  return running;
}

void System::run() {
  // init
  Globals::init();
  
  // all system threads
  Thread<SystemSpeak> speak;
  Thread<SystemListen> listen;
  Thread<SystemDetectFailure> detectFailure;
  
  // start all threads
  speak.start();
  listen.start();
  detectFailure.start();
  
  // join all threads
  speak.join();
  listen.join();
  detectFailure.join();
  
  // close
  Globals::close();
}
