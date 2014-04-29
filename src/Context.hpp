/*
 * Context.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <SDL.h>
#include <map>

class Context {
  public:
    class Image {
      private:
        SDL_Texture* texture;
      public:
        Image(const std::string& fn);
        ~Image();
        void render(int x, int y);
    };
    
    enum InputState {
      PRESSED,
      JUST_PRESSED,
      RELEASED,
      JUST_RELEASED
    };
  private:
    static std::map<SDL_Keycode, InputState> keys;
    static SDL_Window* window;
    static SDL_Renderer* renderer;
    static bool quit;
  public:
    static void init(const char* title, int w, int h, const char* icon);
    static void close();
    static bool shouldQuit();
    static void input();
    static void render();
    static InputState key(SDL_Keycode keycode);
};

#endif /* CONTEXT_HPP_ */
