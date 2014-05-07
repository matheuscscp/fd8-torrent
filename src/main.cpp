/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include "Context.hpp"
#include "Platform.hpp"
#include "System.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  Context::init("..::fd8-torrent::..", 512, 512, "img/icon.jpg");
  
  Context::Image bg("img/bg.jpg");
  Context::Image start("img/start.png");
  Context::Image stop("img/stop.png");
  Context::Image browse("img/browse.png");
  
  while (!Context::shouldQuit()) {
    Context::input();
    
    // update
    if (start.leftClicked()) {
      if (!System::start())
        System::stop();
    }
    if (System::isRunning() && browse.leftClicked())
      Platform::browser();
    
    // render
    bg.render(0, 0);
    if (!System::isRunning())
      start.render(155, 300);
    else {
      stop.render(155, 300);
      browse.render(205, 102);
    }
    
    Context::render();
  }
  
  System::stop();
  Context::close();
  
  return 0;
}
