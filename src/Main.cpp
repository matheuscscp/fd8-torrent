/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include "Context.hpp"
#include "System.hpp"
#include "Helpers.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  Context::init("..::fd8-torrent::..", 512, 512, "img/icon.jpg");
  
  Context::Image bg("img/bg.jpg");
  Context::Image start("img/start.png");
  Context::Image stop("img/stop.png");
  Context::Image browse("img/browse.png");
  
  while (!Context::quitRequested()) {
    Context::input();
    
    // update
    if (start.leftClicked() && !System::changing()) {
      if (!System::start())
        System::stop();
    }
    if (browse.leftClicked() && System::running())
      helpers::openBrowser();
    
    // render
    bg.render(0, 0);
    if (System::running()) {
      stop.render(155, 300);
      browse.render(205, 102);
    }
    else
      start.render(155, 300);
    
    Context::render();
  }
  
  System::stop(true);
  Context::close();
  
  return 0;
}
