/*
 * Thread.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef THREAD_HPP_
#define THREAD_HPP_

// standard
#include <functional>

// lib
#include <SDL.h>

class Thread {
  private:
    std::function<void()> f;
    bool started;
    SDL_Thread* thread;
  public:
    Thread(std::function<void()> f);
    
    void start();
    void join();
    
    static void sleep(Uint32 ms, const bool* keepCondition = nullptr);
  private:
    static int exec(void* func);
};

#endif /* THREAD_HPP_ */
