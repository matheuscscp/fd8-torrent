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

void System() {
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
