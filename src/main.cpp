/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include "Context.hpp"
#include "System.hpp"
#include "Platform.hpp"

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
    if (Context::button(Context::LEFT_MOUSE_BUTTON) == Context::JUST_PRESSED &&
        start.isMouseInside()) {
      if (!System::start())
        System::stop();
    }
    if (System::isRunning() &&
        Context::button(Context::LEFT_MOUSE_BUTTON) == Context::JUST_PRESSED &&
        browse.isMouseInside())
      browser();
    
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
