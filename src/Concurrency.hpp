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
    void* thread;
    bool joined;
    bool* terminated;
  public:
    Thread(const std::function<void()>& f);
    ~Thread();
    Thread(const Thread& other);
    Thread& operator=(const Thread& other);
    Thread(Thread&& other);
    Thread& operator=(Thread&& other);
    
    void start();
    void join();
    bool running();
    
    static void sleep(uint32_t ms, bool* keepCondition = nullptr);
  private:
    static int exec(void* threadInfo);
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
