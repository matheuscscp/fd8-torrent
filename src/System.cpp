/*
 * System.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "Globals.hpp"
#include "SystemDetectFailure.hpp"
#include "SystemListen.hpp"
#include "SystemSpeak.hpp"
#include "SystemWebServer.hpp"

bool System::isRunning = false;
Thread* System::thread = nullptr;

bool System::start() {
  if (isRunning)
    return false;
  isRunning = true;
  
  thread = new Thread(System::run);
  thread->start();
  return true;
}

bool System::stop() {
  if (!isRunning)
    return false;
  isRunning = false;
  
  Globals::get<bool>("systemOn").value() = false;
  thread->join();
  delete thread;
  return true;
}

bool System::running() {
  return isRunning;
}

void System::run() {
  Globals::init();
  bool& systemOn = Globals::get<bool>("systemOn").value();
  while (systemOn) {
    SystemSpeak();
    SystemListen();
    SystemDetectFailure();
    //SystemWebServer();
    Thread::sleep(MS_SLEEP);
  }
  Globals::close();
}
