/*
 * Network.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: Pimenta
 */

// this
#include "Network.hpp"

// standard
#include <cstdio>

// lib
#include <SDL_net.h>
#ifdef _WIN32
  #include <ws2tcpip.h>
#else
  #include <netinet/in.h>
  #include <unistd.h>
#endif

using namespace std;
using namespace helpers;

namespace network {

// =============================================================================
// functions
// =============================================================================

void init() {
  if (SDLNet_Init()) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(0);
  }
}

void close() {
  SDLNet_Quit();
}

// =============================================================================
// class Address;
// =============================================================================

Address::Address(uint32_t ip, uint16_t port) : ip(ip), port(port) {
  
}

Address::Address(const string& ip, const string& port) : ip(0), port(0) {
  if (ip.size()) {
    IPaddress tmp;
    SDLNet_ResolveHost(&tmp, ip.c_str(), 0);
    this->ip = tmp.host;
  }
  if (port.size()) {
    sscanf(port.c_str(), "%hu", &this->port);
    this->port = htons(this->port);
  }
}

Address Address::local() {
  IPaddress addrs[100];
  int total = SDLNet_GetLocalAddresses(addrs, 100);
  int i;
  for (i = 0; i < total && addrs[i].host == 0x0100007F; ++i);
  IPaddress addr;
  SDLNet_ResolveHost(&addr, SDLNet_ResolveIP(&addrs[i]), 0);
  return Address(addr.host, 0);
}

string Address::toString() const {
  char tmp[30];
  if (port != 0) {
    sprintf(tmp, "%d.%d.%d.%d:%d",
      int(((uint8_t*)&ip)[0]),
      int(((uint8_t*)&ip)[1]),
      int(((uint8_t*)&ip)[2]),
      int(((uint8_t*)&ip)[3]),
      ntohs(port)
    );
  }
  else {
    sprintf(tmp, "%d.%d.%d.%d",
      int(((uint8_t*)&ip)[0]),
      int(((uint8_t*)&ip)[1]),
      int(((uint8_t*)&ip)[2]),
      int(((uint8_t*)&ip)[3])
    );
  }
  return string(tmp);
}

uint32_t Address::ntohl(uint32_t ip) {
  return SDLNet_Read32(&ip);
}

uint32_t Address::htonl(uint32_t ip) {
  return SDLNet_Read32(&ip);
}

uint16_t Address::ntohs(uint16_t port) {
  return SDLNet_Read16(&port);
}

uint16_t Address::htons(uint16_t port) {
  return SDLNet_Read16(&port);
}

// =============================================================================
// class UDPSocket;
// =============================================================================

UDPSocket::UDPSocket(size_t maxlen) : maxlen(maxlen) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(ULONG));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

UDPSocket::UDPSocket(const string& port, size_t maxlen) : maxlen(maxlen) {
  Address nport("", port);
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(ULONG));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = nport.port;
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = nport.port;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

UDPSocket::UDPSocket(const Address& multicastAddress, size_t maxlen) : maxlen(maxlen) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(ULONG));
  ip_mreq mreq;
  mreq.imr_multiaddr.S_un.S_addr = multicastAddress.ip;
  mreq.imr_interface.S_un.S_addr = Address::local().ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = multicastAddress.port;
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = multicastAddress.ip;
  mreq.imr_interface.s_addr = Address::local().ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = multicastAddress.port;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

UDPSocket::~UDPSocket() {
#ifdef _WIN32
  closesocket(sd);
#else
  ::close(sd);
#endif
}

void UDPSocket::send(const Address& address, const ByteQueue& data) {
#ifdef _WIN32
  SOCKADDR_IN servaddr;
  memset((void*)&servaddr, 0, sizeof(SOCKADDR_IN));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = address.ip;
  servaddr.sin_port = address.port;
  sendto(sd, (const char*)data.ptr(), data.size(), 0, (SOCKADDR*)&servaddr, sizeof(SOCKADDR_IN));
#else
  sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(sockaddr_in));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = address.ip;
  servaddr.sin_port = address.port;
  sendto(sd, data.ptr(), data.size(), 0, (const sockaddr*)&servaddr, sizeof(sockaddr_in));
#endif
}

ByteQueue UDPSocket::recv(Address& address) {
  ByteQueue data;
#ifdef _WIN32
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(0, &fds, nullptr, nullptr, &timeout) > 0) {
    SOCKADDR_IN addr;
    int addrsize = sizeof(SOCKADDR_IN);
    data.resize(maxlen);
    data.resize(size_t(recvfrom(SOCKET(sd), (char*)data.ptr(), maxlen, 0, (SOCKADDR*)&addr, &addrsize)));
    address = Address(addr.sin_addr.S_un.S_addr, addr.sin_port);
  }
#else
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(sd + 1, &fds, nullptr, nullptr, &timeout) > 0) {
    sockaddr_in addr;
    socklen_t addrsize = sizeof(sockaddr_in);
    data.resize(maxlen);
    data.resize(size_t(recvfrom(sd, data.ptr(), maxlen, 0, (sockaddr*)&addr, &addrsize)));
    address = Address(addr.sin_addr.s_addr, addr.sin_port);
  }
#endif
  return data;
}

// =============================================================================
// class TCPSocket;
// =============================================================================

TCPSocket::TCPSocket() : sd(0) {
  
}

TCPSocket::~TCPSocket() {
  if (sd)
    SDLNet_TCP_Close(TCPsocket(sd));
}

// =============================================================================
// class TCPConnection;
// =============================================================================

TCPConnection::TCPConnection(const Address& addr) {
  if (addr.port) {
    IPaddress tmp;
    tmp.host = addr.ip;
    tmp.port = addr.port;
    sd = SDLNet_TCP_Open(&tmp);
  }
}

void TCPConnection::send(const ByteQueue& data) {
  if (sd)
    SDLNet_TCP_Send(TCPsocket(sd), data.ptr(), data.size());
}

void TCPConnection::send(const void* data, size_t maxlen) {
  if (sd)
    SDLNet_TCP_Send(TCPsocket(sd), data, maxlen);
}

void TCPConnection::send(const string& data, bool withoutNullTermination) {
  if (sd) {
    ByteQueue tmp;
    tmp.push(data, withoutNullTermination);
    SDLNet_TCP_Send(TCPsocket(sd), (const void*)tmp.ptr(), tmp.size());
  }
}

void TCPConnection::recv(ByteQueue& data) {
  size_t total = 0;
  if (sd)
    total = SDLNet_TCP_Recv(TCPsocket(sd), data.ptr(), data.size());
  data.resize(total);
}

size_t TCPConnection::recv(void* data, size_t maxlen) {
  size_t total = 0;
  if (sd)
    total = SDLNet_TCP_Recv(TCPsocket(sd), data, maxlen);
  return total;
}

string TCPConnection::recv(size_t maxlen) {
  string data;
  if (sd) {
    data.resize(maxlen, '0');
    data.resize(SDLNet_TCP_Recv(TCPsocket(sd), (void*)data.c_str(), maxlen));
  }
  return data;
}

// =============================================================================
// class TCPServer;
// =============================================================================

TCPServer::TCPServer(const string& port) {
  if (port.size()) {
    IPaddress addr;
    SDLNet_ResolveHost(&addr, nullptr, Address::ntohs(Address("", port).port));
    sd = SDLNet_TCP_Open(&addr);
  }
}

TCPConnection* TCPServer::accept() {
  TCPConnection* conn = nullptr;
  if (sd) {
    TCPsocket newsd = SDLNet_TCP_Accept(TCPsocket(sd));
    if (newsd) {
      conn = new TCPConnection(Address(0, 0));
      ((TCPServer*)conn)->sd = newsd;
    }
  }
  return conn;
}

} // namespace network
