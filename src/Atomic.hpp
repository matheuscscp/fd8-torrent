/*
 * Atomic.hpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

#ifndef ATOMIC_HPP_
#define ATOMIC_HPP_

// lib
#include <memory>

// local
#include "Mutex.hpp"

class AtomicBase {
    
};

template <class T>
class Atomic : public AtomicBase {
  private:
    std::unique_ptr<T> val;
    Mutex mutex;
  public:
    Atomic(T* val) : val(val) {
      
    }
    
    ~Atomic() {
      
    }
    
    void lock() {
      mutex.lock();
    }
    
    void unlock() {
      mutex.unlock();
    }
    
    T& value() const {
      return *(val.get());
    }
};

#endif /* ATOMIC_HPP_ */
