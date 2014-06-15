/*
 * Concurrency.hpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

#ifndef CONCURRENCY_HPP_
#define CONCURRENCY_HPP_

// standard
#include <functional>
#include <memory>

namespace concurrency {

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

class Mutex {
  private:
    void* mutex;
  public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
};

class AtomicBase {
  public:
    virtual ~AtomicBase() = 0;
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

} // namespace concurrency

#endif /* CONCURRENCY_HPP_ */
