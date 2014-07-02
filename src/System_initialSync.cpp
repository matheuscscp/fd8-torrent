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
  // find someone to send the system state to this peer
  uint32_t ip = 0;
  for (auto& kv : users) {
    if (kv.first != localAddress.ip) {
      ip = kv.first;
      break;
    }
  }
  
  User& user = users[localAddress.ip];
  
  if (!ip) { // no one found, then return
    user.sessionID = 1;
    nextSessionID = 2;
    return;
  }
  
  // ask for synchronization
  TCPConnection conn(Address(ip, Address("", TCPUDP_MAIN).port));
  conn.send(char(MTYPE_SYNC));
  user.sessionID = conn.recv<uint32_t>();
  nextSessionID = user.sessionID + 1;
  ByteQueue data(conn.recv<uint32_t>());
  conn.recv(data);
  FileSystem::deserialize(data);
}

void System::respondSystemState(TCPConnection* peer) {
  peer->send(nextSessionID);
  ByteQueue data = FileSystem::serialize();
  peer->send(uint32_t(data.size()));
  peer->send(data);
}
