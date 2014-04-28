/*
 * Mutex.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

#ifndef MUTEX_HPP_
#define MUTEX_HPP_

class Mutex {
  private:
    void* mutex;
  public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
};

#endif /* MUTEX_HPP_ */
