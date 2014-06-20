/*
 * File.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

#ifndef FILE_HPP_
#define FILE_HPP_

// standard
#include <cstdint>

struct File {
  public:
    int size;
    uint32_t peer1;
    uint32_t peer2;
};

#endif /* FILE_HPP_ */
