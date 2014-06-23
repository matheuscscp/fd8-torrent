/*
 * System_stateInit.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <vector>
#include <cstdio>

// local
#include "Network.hpp"
#include "Defines.hpp"
#include "FD8Protocol.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace network;
using namespace fd8protocol;
using namespace helpers;

void System::stateInit() {
  Address addr;
  if (mainUDPSocket.recv(addr).size()) {
    // open connection with the peer
    TCPConnection conn(Address(addr.ip, Address("", TCPUDP_MAIN).port));
    
    conn.send(char(MTYPE_SYNC));
    
    // receive user table
    uint32_t userAmount = conn.recv<uint32_t>();
    for (uint32_t i = 0; i < userAmount; i++) {
      uint32_t ip = conn.recv<uint32_t>();
      users[ip] = User(conn.recv<string>());
    }
    
    // receive file system
    {
      ByteQueue data(conn.recv<uint32_t>());
      conn.recv(data);
      FileSystem::deserialize(data);
    }
    
    changeToLogin();
  }
  else if (initTimer.time() > MS_INITTIMER) {
    changeToLogin();
  }
}
