/*
 * Context.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

// this
#include "Context.hpp"

// standard
#include <cstdio>
#include <cstdlib>

// lib
#include <SDL_image.h>
#include <SDL_net.h>

using namespace std;

// =============================================================================
// Context
// =============================================================================

map<SDL_Keycode, Context::InputState> Context::keys;
SDL_Window* Context::window = nullptr;
SDL_Renderer* Context::renderer = nullptr;
bool Context::quit = false;

void Context::init(const char* title, int w, int h, const char* icon) {
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
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (icon)
    SDL_SetWindowIcon(window, IMG_Load(icon));
}

void Context::close() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDLNet_Quit();
  IMG_Quit();
  SDL_Quit();
}

bool Context::shouldQuit() {
  return quit;
}

void Context::input() {
  for (auto& kv : keys) {
    if (kv.second == JUST_PRESSED)
      kv.second = PRESSED;
    else if (kv.second == JUST_RELEASED)
      kv.second = RELEASED;
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
        
      case SDL_QUIT:
        quit = true;
        break;
        
      default:
        break;
    }
  }
}

void Context::render() {
  SDL_RenderPresent(renderer);
}

Context::InputState Context::key(SDL_Keycode keycode) {
  if (keys.find(keycode) == keys.end()) {
    keys[keycode] = RELEASED;
    return RELEASED;
  }
  return keys[keycode];
}

// =============================================================================
// Context::Image
// =============================================================================

Context::Image::Image(const string& fn) {
  texture = IMG_LoadTexture(Context::renderer, fn.c_str());
}

Context::Image::~Image() {
  SDL_DestroyTexture(texture);
}

void Context::Image::render(int x, int y) {
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
}
