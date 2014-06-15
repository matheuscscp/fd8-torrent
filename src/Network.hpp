/*
 * Network.hpp
 *
 *  Created on: Jun 14, 2014
 *      Author: Pimenta
 */

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

// standard
#include <vector>
#include <string>
#include <cstdint>

namespace network {

// network endianness
struct Address {
  uint32_t ip;
  uint16_t port;
  Address();
  Address(uint32_t ip, uint16_t port);
  Address(const std::string& ip, uint16_t port); // port in host order
  std::string toString() const;
  static Address local();
  static uint32_t ntohl(uint32_t ip);
  static uint32_t htonl(uint32_t ip);
  static uint16_t ntohs(uint16_t port);
  static uint16_t htons(uint16_t port);
};

class UDPSocket {
  private:
    int sd;
  public:
    UDPSocket(uint16_t port);
    UDPSocket(const Address& address, uint32_t group);
    ~UDPSocket();
    void send(const Address& address, const std::vector<char>& data);
    std::vector<char> recv(Address& address);
};

class TCPServer {
  private:
    
};

}

#endif /* NETWORK_HPP_ */
