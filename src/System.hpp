/*
 * System.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include "Thread.hpp"

class System {
  private:
    static void run();
    static Thread<run>* thread;
    static bool running;
  public:
    static bool start();
    static bool stop();
    static bool isRunning();
  private:
};

#endif /* SYSTEM_HPP_ */
