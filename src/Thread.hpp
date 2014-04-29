/*
 * Thread.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef THREAD_HPP_
#define THREAD_HPP_

// standard
#include <cstdio>

// lib
#include <SDL_thread.h>

typedef void (*Callback)();

template <Callback cb>
class Thread {
  private:
    bool running;
    SDL_Thread* thread;
  public:
    Thread() : running(false), thread(nullptr) {
      
    }
    
    void start() {
      if (running)
        return;
      char name[20];
      sprintf(name, "%p", (void*)cb);
      thread = SDL_CreateThread(threadCallback, name, &running);
    }
    
    void join() {
      SDL_WaitThread(thread, nullptr);
    }
    
    bool isRunning() {
      return running;
    }
  private:
    static int threadCallback(void* running) {
      *((bool*)running) = true;
      cb();
      *((bool*)running) = false;
      return 0;
    }
};

void Thread_sleep(Uint32 ms, const bool* keepCondition);
void Thread_sleep(Uint32 ms);

#endif /* THREAD_HPP_ */
