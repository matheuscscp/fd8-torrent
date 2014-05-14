/*
 * Platform.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef PLATFORM_HPP_
#define PLATFORM_HPP_

// standard
#include <cstdint>
#include <vector>

namespace platform {

class MulticastSocket {
  private:
    int sd;
    uint32_t ip;
  public:
    MulticastSocket(uint32_t ip, uint16_t port, uint32_t group);
    ~MulticastSocket();
    std::vector<char> read(uint32_t& host, uint16_t& port);
};

void openBrowser();
uint32_t getLocalIP();

}

#endif /* PLATFORM_HPP_ */
