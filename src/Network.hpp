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

struct Address {
  uint32_t ip;
  uint16_t port;
  
  Address(uint32_t ip = 0, uint16_t port = 0);
  Address(const std::string& ip, const std::string& port = "");
  static Address local();
  
  std::string toString() const;
  static uint32_t ntohl(uint32_t ip);
  static uint32_t htonl(uint32_t ip);
  static uint16_t ntohs(uint16_t port);
  static uint16_t htons(uint16_t port);
};

class UDPSocket {
  private:
    int sd;
  public:
    UDPSocket(const std::string& port);
    UDPSocket(const Address& multicastAddress);
    ~UDPSocket();
    void send(const Address& address, const std::vector<char>& data);
    std::vector<char> recv(Address& address);
};

class TCPSocket {
  protected:
    void* sd;
  public:
    TCPSocket(void* sd);
    virtual ~TCPSocket() = 0;
};

class TCPConnection : public TCPSocket {
  public:
    TCPConnection(void* sd);
    void send(const std::vector<char>& data);
    std::vector<char> recv(int maxlen);
};

class TCPServer : public TCPSocket {
  public:
    TCPServer(const std::string& port);
    TCPConnection accept();
};

}

#endif /* NETWORK_HPP_ */
