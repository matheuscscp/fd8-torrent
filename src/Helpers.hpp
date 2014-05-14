/*
 * Helpers.hpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

#ifndef HELPERS_HPP_
#define HELPERS_HPP_

// standard
#include <cstdint>
#include <string>

namespace helpers {

uint32_t str2Network(const char* str);
std::string network2str(uint32_t ipaddr);

}

#endif /* HELPERS_HPP_ */
