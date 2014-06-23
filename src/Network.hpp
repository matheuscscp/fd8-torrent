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

// local
#include "Helpers.hpp"

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
    size_t maxlen;
  public:
    UDPSocket(size_t maxlen);
    UDPSocket(const std::string& port, size_t maxlen);
    UDPSocket(const Address& multicastAddress, size_t maxlen);
    ~UDPSocket();
    void send(const Address& address, const helpers::ByteQueue& data);
    helpers::ByteQueue recv(Address& address);
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
    
    void send(const helpers::ByteQueue& data);
    void send(const void* data, size_t maxlen);
    void send(const std::string& data, bool withoutNullTermination = false);
    template <typename T> void send(T data) {
      send((const void*)&data, sizeof(T));
    }
    
    void recv(helpers::ByteQueue& data);
    size_t recv(void* data, size_t maxlen);
    std::string recv(size_t maxlen);
    template <typename T> T recv() {
      T data = 0;
      recv(&data, sizeof(T));
      return data;
    }
};

template <> inline void TCPConnection::send<std::string>(std::string data) {
  send(data, false);
}

template <> inline std::string TCPConnection::recv<std::string>() {
  std::string data;
  char c;
  while ((c = recv<char>()) != '\0')
    data += c;
  return data;
}

class TCPServer : public TCPSocket {
  public:
    TCPServer(const std::string& port);
    TCPConnection* accept();
};

} // namespace network

#endif /* NETWORK_HPP_ */
