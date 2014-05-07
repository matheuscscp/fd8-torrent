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

class Thread {
  private:
    std::function<void()> f;
    bool started;
    void* thread;
  public:
    Thread(const std::function<void()>& f);
    
    void start();
    void join();
    
    static void sleep(uint32_t ms, const bool* keepCondition = nullptr);
  private:
    static int exec(void* func);
};

#endif /* THREAD_HPP_ */
