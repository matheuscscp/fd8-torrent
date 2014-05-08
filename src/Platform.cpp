/*
 * Platform.cpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

// this
#include "Platform.hpp"

// standard
#include <cstdlib>
#include <cstdio>

// lib
#ifdef _WIN32
  #include <ws2tcpip.h>
#else
  #include <netinet/in.h>
  #include <unistd.h>
#endif

// local
#include "Defines.hpp"

using namespace std;

namespace platform {

// =============================================================================
// platform::MulticastSocket
// =============================================================================

MulticastSocket::MulticastSocket(uint32_t ip, uint16_t port, uint32_t group) : ip(ip) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)TRUE, sizeof(BOOL));
  ip_mreq mreq;
  mreq.imr_multiaddr.S_un.S_addr = group;
  mreq.imr_interface.S_un.S_addr = ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = group;
  mreq.imr_interface.s_addr = ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

MulticastSocket::~MulticastSocket() {
#ifdef _WIN32
  closesocket(sd);
#else
  close(sd);
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
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(sd + 1, &fds, nullptr, nullptr, &timeout) > 0) {
    char buf[0x1000];
    sockaddr_in addr;
    socklen_t addrsize = sizeof(sockaddr_in);
    int total = recvfrom(sd, buf, 0x1000, 0, (sockaddr*)&addr, &addrsize);
    if (addr.sin_addr.s_addr != ip) {
      data.assign(buf, buf + total);
      host = addr.sin_addr.s_addr;
      port = addr.sin_port;
    }
  }
#endif
  return data;
}

// =============================================================================
// platform functions
// =============================================================================

void browser() {
  char cmd[100];
#ifdef _WIN32
  sprintf(cmd, "start http://localhost:%d", TCP_WEBSERVER);
#else
  sprintf(cmd, "sensible-browser http://localhost:%d", TCP_WEB_HOST);
#endif
  system(cmd);
}

} // namespace platform
