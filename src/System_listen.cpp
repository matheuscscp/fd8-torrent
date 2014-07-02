/*
 * System_listen.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "FD8Protocol.hpp"

using namespace std;
using namespace network;
using namespace helpers;
using namespace fd8protocol;

void System::listen() {
  // listening
  Address addr;
  ByteQueue data;
  data = mainUDPSocket.recv(addr);
  if (!data.size()) // no beacon was received
    return;
  
  uint32_t sessionID = data.pop<uint32_t>();
  string userName = data.pop<string>();
  
  // ignoring beacons sent from this peer and non-private network addresses
  if (userName == users[localAddress.ip].name || !addr.isPrivateNetwork())
    return;
  
  auto userIt = users.find(addr.ip);
  if (userIt != users.end()) { // IP already in the user table
    if (userIt->second.sessionID == sessionID) // refresh beacon
      userIt->second.timer.start();
    else { // someone logged out and then logged in, failure detected
      users.erase(userIt);
      if (state == STATE_IDLE)
        recoverFromFailure();
      User& user = users[addr.ip];
      user.sessionID = sessionID;
      user.name = userName;
      user.timer.start();
      nextSessionID = user.sessionID + 1;
    }
  }
  else { // IP just logged in
    User& user = users[addr.ip];
    user.sessionID = sessionID;
    user.name = userName;
    user.timer.start();
    nextSessionID = user.sessionID + 1;
  }
}
