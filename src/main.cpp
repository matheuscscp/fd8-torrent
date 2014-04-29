/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include "Context.hpp"
#include "System.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  Context::init("fd8-torrent", 512, 512, "img/icon.jpg");
  
  Context::Image bg("img/bg.jpg");
  
  while (!Context::shouldQuit()) {
    Context::input();
    
    // update
    if (Context::key(SDLK_a) == Context::JUST_PRESSED) {
      if (!System::start())
        System::stop();
      printf("system status: %d\n", (int)System::isRunning());
    }
    
    // render
    bg.render(0, 0);
    
    Context::render();
  }
  
  System::stop();
  Context::close();
  
  return 0;
}
