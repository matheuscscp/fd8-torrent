/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include <SDL_net.h>
#include <cstdio>
#include <cstdlib>

#include "Globals.hpp"
#include "System.hpp"
#include "Thread.hpp"

using namespace std;

enum KeyState {
  PRESSED,
  JUST_PRESSED,
  RELEASED,
  JUST_RELEASED
};

static KeyState keys[300];

static void init() {
  if (SDL_Init(0)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    exit(0);
  }
  if (SDLNet_Init()) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(0);
  }
}

static void close() {
  SDLNet_Quit();
  SDL_Quit();
}

static void input() {
  for (auto& i : keys) {
    if (i == JUST_PRESSED)
      i = PRESSED;
    else if (i == JUST_RELEASED)
      i = RELEASED;
  }
  
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        keys[event.key.keysym.sym] = JUST_PRESSED;
        break;
      case SDL_KEYUP:
        keys[event.key.keysym.sym] = JUST_RELEASED;
        break;
      default:
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  init();
  
  SDL_Window* window = SDL_CreateWindow("fd8-torrent", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 480, 0);
  
  bool systemOn = false;
  Thread<System> systemThread;
  while (!SDL_QuitRequested()) {
    input();
    
    if (keys[SDLK_a] == JUST_PRESSED) {
      systemOn = !systemOn;
      if (!systemOn) {
        Globals::get<bool>("systemOn").value() = false;
        systemThread.join();
      }
      else
        systemThread.start();
      printf("system status: %d\n", (int)systemOn);
    }
    
    Thread_sleep(50);
  }
  
  SDL_DestroyWindow(window);
  
  close();
  return 0;
}
