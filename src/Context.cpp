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

map<SDL_Keycode, pair<bool, Context::InputState>> Context::keys;
Context::InputState Context::buttons[6];
int Context::mousex;
int Context::mousey;
int Context::mousedownx = -1;
int Context::mousedowny = -1;
int Context::mouseupx = -1;
int Context::mouseupy = -1;
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
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (icon) {
    SDL_Surface* surface = IMG_Load(icon);
    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);
  }
  for (auto& butt : buttons)
    butt = RELEASED;
}

void Context::close() {
  closeWindow();
  IMG_Quit();
  SDL_Quit();
}

void Context::closeWindow() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
}

bool Context::quitRequested() {
  return quit;
}

void Context::input() {
  for (auto& kv : keys) {
    if (kv.second.second == JUST_PRESSED)
      kv.second.second = PRESSED;
    else if (kv.second.second == JUST_RELEASED)
      kv.second.second = RELEASED;
  }
  
  SDL_GetMouseState(&mousex, &mousey);
  for (auto& i : buttons) {
    if (i == JUST_PRESSED)
      i = PRESSED;
    else if (i == JUST_RELEASED)
      i = RELEASED;
  }
  
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        if (!keys[event.key.keysym.sym].first) {
          keys[event.key.keysym.sym].first = true;
          keys[event.key.keysym.sym].second = JUST_PRESSED;
        }
        break;
        
      case SDL_KEYUP:
        keys[event.key.keysym.sym].first = false;
        keys[event.key.keysym.sym].second = JUST_RELEASED;
        break;
        
      case SDL_MOUSEBUTTONDOWN:
        buttons[event.button.button] = JUST_PRESSED;
        mousedownx = mousex;
        mousedowny = mousey;
        break;
        
      case SDL_MOUSEBUTTONUP:
        buttons[event.button.button] = JUST_RELEASED;
        mouseupx = mousex;
        mouseupy = mousey;
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
  auto it = keys.find(keycode);
  if (it == keys.end()) {
    keys[keycode].first = false;
    keys[keycode].second = RELEASED;
    return RELEASED;
  }
  return it->second.second;
}

Context::InputState Context::button(int butt) {
  return buttons[butt];
}

// =============================================================================
// Context::Image
// =============================================================================

Context::Image::Image(const string& fn) {
  texture = IMG_LoadTexture(Context::renderer, fn.c_str());
  SDL_QueryTexture(texture, nullptr, nullptr, &dstrect.w, &dstrect.h);
}

Context::Image::~Image() {
  SDL_DestroyTexture(texture);
}

void Context::Image::render(int x, int y) {
  dstrect.x = x;
  dstrect.y = y;
  SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
}

bool Context::Image::leftClicked() {
  return
    button(LEFT_MOUSE_BUTTON) == JUST_RELEASED &&
    mousedownx >= dstrect.x && mousedownx < dstrect.x + dstrect.w &&
    mousedowny >= dstrect.y && mousedowny < dstrect.y + dstrect.h &&
    mouseupx >= dstrect.x && mouseupx < dstrect.x + dstrect.w &&
    mouseupy >= dstrect.y && mouseupy < dstrect.y + dstrect.h
  ;
}
