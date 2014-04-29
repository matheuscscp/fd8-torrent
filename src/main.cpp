/*
 * main.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: Pimenta
 */

#include <SDL_image.h>
#include <SDL_net.h>
#include <cstdio>
#include <cstdlib>

#include "System.hpp"

using namespace std;

enum KeyState {
  PRESSED,
  JUST_PRESSED,
  RELEASED,
  JUST_RELEASED
};

static KeyState keys[300];

static void init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    fprintf(stderr, "SDL_Init failure: %s\n", SDL_GetError());
    exit(0);
  }
  if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF) != (IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF)) {
    fprintf(stderr, "IMG_Init failure\n");
    exit(0);
  }
  if (SDLNet_Init()) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(0);
  }
}

static void close() {
  SDLNet_Quit();
  IMG_Quit();
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
  
  while (!SDL_QuitRequested()) {
    input();
    
    if (keys[SDLK_a] == JUST_PRESSED) {
      if (!System::start())
        System::stop();
      printf("system status: %d\n", (int)System::isRunning());
    }
    
    Thread_sleep(50);
  }
  
  System::stop();
  SDL_DestroyWindow(window);
  
  close();
  return 0;
}
