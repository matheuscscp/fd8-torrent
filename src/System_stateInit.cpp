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

// local
#include "Network.hpp"
#include "Defines.hpp"
#include "FD8Protocol.hpp"

using namespace std;
using namespace network;
using namespace fd8protocol;

void System::stateInit() {
  Address addr;
  vector<char> data = mainUDPSocket.recv(addr);
  if (data.size()) {
    // open connection with the peer
    TCPConnection conn(Address(addr.ip, Address("", TCPUDP_MAIN).port));
    
    // send "get users" message
    conn.send(char(MTYPE_GETUSERS));
    
    // receive response
    uint32_t userAmount = conn.recv<uint32_t>();
    for (uint32_t i = 0; i < userAmount; i++) {
      uint32_t ip = conn.recv<uint32_t>();
      //users[ip] = User
    }
    state = STATE_LOGIN;
  }
}
