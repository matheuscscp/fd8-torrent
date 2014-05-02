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
#include "Lockable.hpp"
#include "SystemDetectFailure.hpp"
#include "SystemListen.hpp"
#include "SystemSpeak.hpp"
#include "SystemWeb.hpp"

Thread* System::thread = nullptr;
bool System::running = false;

bool System::start() {
  if (running)
    return false;
  
  thread = new Thread(System::run);
  thread->start();
  running = true;
  return true;
}

bool System::stop() {
  if (!running)
    return false;
  
  Globals::get<bool>("systemOn").value() = false;
  thread->join();
  running = false;
  return true;
}

bool System::isRunning() {
  return running;
}

void System::run() {
  // init
  Globals::init();
  
  // all system threads
  Thread speak(SystemSpeak);
  Thread listen(SystemListen);
  Thread detectFailure(SystemDetectFailure);
  Thread web(SystemWeb);
  
  // start all threads
  speak.start();
  listen.start();
  detectFailure.start();
  web.start();
  
  // join all threads
  speak.join();
  listen.join();
  detectFailure.join();
  web.join();
  
  // close
  Globals::close();
}
