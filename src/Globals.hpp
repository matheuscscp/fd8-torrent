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
#include "Lockable.hpp"

class Globals {
  private:
    static std::map<std::string, LockableBase*> globals;
    static bool isInit;
  public:
    static void init();
    static void close();
    
    template <class T>
    static Lockable<T>& get(const std::string& key) {
      return *((Lockable<T>*)globals[key]);
    }
};

#endif /* GLOBALS_HPP_ */
