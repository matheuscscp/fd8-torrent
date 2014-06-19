/*
 * User.hpp
 *
 *  Created on: 19/06/2014
 *      Author: Joao Alves
 */

#ifndef USER_HPP_
#define USER_HPP_

// standard
#include <string>

// local
#include "Helpers.hpp"

class User {
  public:
    std::string name;
    helpers::Timer timer;
    User(const std::string& name);
};

#endif /* USER_HPP_ */
