/*
 * Platform.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef PLATFORM_HPP_
#define PLATFORM_HPP_

#include <cstdlib>

#ifdef _WIN32

#define browser() system("start http://localhost:8080")

#else

#define browser() system("sensible-browser http://localhost:8080")

#endif

#endif /* PLATFORM_HPP_ */
