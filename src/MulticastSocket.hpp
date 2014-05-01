/*
 * MulticastSocket.hpp
 *
 *  Created on: May 1, 2014
 *      Author: Pimenta
 */

#ifndef MULTICASTSOCKET_HPP_
#define MULTICASTSOCKET_HPP_

// standard
#include <cstdint>
#include <vector>

class MulticastSocket {
  private:
    int sd;
    uint32_t ip;
  public:
    MulticastSocket(uint32_t ip, uint16_t port, uint32_t group);
    ~MulticastSocket();
    std::vector<char> read(uint32_t& host, uint16_t& port);
};

#endif /* MULTICASTSOCKET_HPP_ */
