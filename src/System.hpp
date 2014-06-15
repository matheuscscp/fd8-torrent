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
    static bool isRunning;
    static Thread* thread;
  public:
    static bool start();
    static bool stop();
    static bool running();
  private:
    static void run();
};

#endif /* SYSTEM_HPP_ */
