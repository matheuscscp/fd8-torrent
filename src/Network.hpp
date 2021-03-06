/*
 * Network.hpp
 *
 *  Created on: Jun 14, 2014
 *      Author: Pimenta
 */

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

// standard
#include <cstdio>

// local
#include "Helpers.hpp"
#include "Defines.hpp"

namespace network {

void init();
void close();

class Address {
  public:
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
    
    bool isPrivateNetwork();
    static bool isPrivateNetwork(uint32_t ip);
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
    template <typename T> void send(const std::vector<T>& data) {
      helpers::ByteQueue tmp;
      tmp.push(data);
      send(tmp);
    }
    
    void recv(helpers::ByteQueue& data);
    size_t recv(void* data, size_t maxlen);
    std::string recv(size_t maxlen);
    template <typename T> T recv() {
      T data = 0;
      recv(&data, sizeof(T));
      return data;
    }
    template <typename T> std::vector<T> recv(size_t* size) {
      std::vector<T> data(recv<size_t>());
      if (size)
        *size = data.size();
      recv((void*)&data[0], sizeof(T)*data.size());
      return data;
    }
};

template <> inline void TCPConnection::send<std::string>(std::string data) {
  send(data, false);
}

template <> inline void TCPConnection::send<FILE*>(FILE* data) {
  char buf[SIZE_FILEBUFFER_MAXLEN];
  for (size_t readBytes = 0; (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, data)) > 0; send(buf, readBytes));
}

template <> inline std::string TCPConnection::recv<std::string>() {
  std::string data;
  char c;
  while ((c = recv<char>()) != '\0')
    data += c;
  return data;
}

template <> inline std::vector<std::string> TCPConnection::recv<std::string>(size_t* size) {
  std::vector<std::string> data(recv<size_t>());
  if (size)
    *size = data.size();
  for (auto& str : data)
    str = recv<std::string>();
  return data;
}

class TCPServer : public TCPSocket {
  public:
    TCPServer(const std::string& port);
    TCPConnection* accept();
};

} // namespace network

#endif /* NETWORK_HPP_ */
