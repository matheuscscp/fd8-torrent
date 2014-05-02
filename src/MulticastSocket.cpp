/*
 * MulticastSocket.cpp
 *
 *  Created on: May 1, 2014
 *      Author: Pimenta
 */

// this
#include "MulticastSocket.hpp"

// lib
#ifdef _WIN32

#include <ws2tcpip.h>

#else



#endif

using namespace std;

MulticastSocket::MulticastSocket(uint32_t ip, uint16_t port, uint32_t group) :ip(ip) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)TRUE, sizeof(BOOL));
  ip_mreq mreq;
  mreq.imr_multiaddr.S_un.S_addr = group;
  mreq.imr_interface.S_un.S_addr = ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.S_un.S_addr = INADDR_ANY;
  bind(sd, (sockaddr*)&addr, sizeof(SOCKADDR_IN));
#else
  
#endif
}

MulticastSocket::~MulticastSocket() {
#ifdef _WIN32
  closesocket(sd);
#else
  
#endif
}

vector<char> MulticastSocket::read(uint32_t& host, uint16_t& port) {
  vector<char> data;
#ifdef _WIN32
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(0, &fds, nullptr, nullptr, &timeout) > 0) {
    char buf[0x1000];
    SOCKADDR_IN addr;
    int addrsize = sizeof(SOCKADDR_IN);
    int total = recvfrom(sd, buf, 0x1000, 0, (sockaddr*)&addr, &addrsize);
    if (addr.sin_addr.S_un.S_addr != ip) {
      data.assign(buf, buf + total);
      host = addr.sin_addr.S_un.S_addr;
      port = addr.sin_port;
    }
  }
#else
  
#endif
  return data;
}
