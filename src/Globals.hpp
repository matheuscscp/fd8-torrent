/*
 * Globals.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Pimenta
 */

#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

// standard
#include <map>
#include <string>

// local
#include "Atomic.hpp"

class Globals {
  private:
    static std::map<std::string, AtomicBase*> globals;
    static bool isInit;
  public:
    static void init();
    static void close();
    
    template <class T>
    static Atomic<T>& get(const std::string& key) {
      return *((Atomic<T>*)globals[key]);
    }
};

#endif /* GLOBALS_HPP_ */
