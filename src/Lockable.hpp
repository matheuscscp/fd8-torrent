/*
 * Lockable.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

#ifndef LOCKABLE_HPP_
#define LOCKABLE_HPP_

// local
#include "Mutex.hpp"

class LockableBase {
    
};

template <class T>
class Lockable : public LockableBase {
  private:
    T* val;
    Mutex mutex;
  public:
    Lockable(T* val) : val(val) {
      
    }
    
    ~Lockable() {
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

#endif /* LOCKABLE_HPP_ */
