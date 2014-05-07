/*
 * Atomic.hpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

#ifndef ATOMIC_HPP_
#define ATOMIC_HPP_

// local
#include "Mutex.hpp"

class AtomicBase {
    
};

template <class T>
class Atomic : public AtomicBase {
  private:
    T* val;
    Mutex mutex;
  public:
    Atomic(T* val) : val(val) {
      
    }
    
    ~Atomic() {
      delete val;
    }
    
    void lock() {
      mutex.lock();
    }
    
    void unlock() {
      mutex.unlock();
    }
    
    T& value() const {
      return *val;
    }
};

#endif /* ATOMIC_HPP_ */
