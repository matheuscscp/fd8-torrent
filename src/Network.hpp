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

void init();
void close();

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
    int maxlen;
  public:
    UDPSocket(const std::string& port, int maxlen);
    UDPSocket(const Address& multicastAddress, int maxlen);
    ~UDPSocket();
    void send(const Address& address, const std::vector<char>& data);
    void send(const Address& address, const char* data, int maxlen);
    std::vector<char> recv(Address& address);
};

class TCPSocket {
  protected:
    void* sd;
    TCPSocket();
  public:
    virtual ~TCPSocket();
};

class TCPConnection : public TCPSocket {
  public:
    TCPConnection(const Address& addr);
    void send(const std::vector<char>& data);
    void send(const char* data, int maxlen);
    std::vector<char> recv(int maxlen);
};

class TCPServer : public TCPSocket {
  public:
    TCPServer(const std::string& port);
    TCPConnection* accept();
};

} // namespace network

#endif /* NETWORK_HPP_ */
