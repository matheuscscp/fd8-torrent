/*
 * Context.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

// standard
#include <map>
#include <string>

// lib
#include <SDL.h>

class Context {
  public:
    class Image {
      private:
        SDL_Texture* texture;
        SDL_Rect dstrect;
      public:
        Image(const std::string& fn);
        ~Image();
        void render(int x, int y);
        bool leftClicked();
    };
    
    enum InputState {
      PRESSED,
      JUST_PRESSED,
      RELEASED,
      JUST_RELEASED
    };
    
    enum MouseButton {
      LEFT_MOUSE_BUTTON = SDL_BUTTON_LEFT,
      RIGHT_MOUSE_BUTTON = SDL_BUTTON_RIGHT
    };
  private:
    static std::map<SDL_Keycode, std::pair<bool, InputState>> keys;
    static InputState buttons[6];
    static int mousex, mousey, mousedownx, mousedowny, mouseupx, mouseupy;
    static SDL_Window* window;
    static SDL_Renderer* renderer;
    static bool quit;
  public:
    static void init(const char* title, int w, int h, const char* icon);
    static void close();
    static bool quitRequested();
    static void input();
    static void render();
    static InputState key(SDL_Keycode keycode);
    static InputState button(int butt);
};

#endif /* CONTEXT_HPP_ */
