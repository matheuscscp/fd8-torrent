/*
 * System_initialSync.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

#include "Defines.hpp"
#include "FD8Protocol.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace network;
using namespace fd8protocol;
using namespace helpers;

void System::requestSystemState() {
  Address addr;
  if (!mainUDPSocket.recv(addr).size()) return;
  
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

void System::respondSystemState(TCPConnection* peer) {
  // send user table
  peer->send(uint32_t(users.size()));
  for (auto& kv : users) {
    peer->send(kv.first);
    peer->send(kv.second.name);
  }
  
  // send file system
  {
    ByteQueue data = FileSystem::serialize();
    peer->send(uint32_t(data.size()));
    peer->send(data);
  }
}
