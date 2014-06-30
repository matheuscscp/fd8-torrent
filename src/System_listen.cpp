/*
 * System_listen.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

using namespace std;
using namespace network;
using namespace helpers;

void System::listen() {
  Address addr;
  ByteQueue data;
  data = mainUDPSocket.recv(addr);
  if (!data.size() || addr.ip == localAddress.ip)
    return;
  
  // update timeout
  uint32_t sessionID = data.pop<uint32_t>();
  auto userIt = users.find(addr.ip);
  if (userIt != users.end()) { // IP already in the user table
    if (userIt->second.sessionID == sessionID) // refresh beacon
      userIt->second.timer.start();
    else { // failure detected
      users.erase(userIt);
      recoverFromFailure();
      User& user = users[addr.ip];
      user.sessionID = sessionID;
      user.name = data.pop<string>();
      user.timer.start();
      nextSessionID = user.sessionID + 1;
    }
  }
  else { // IP just logged in
    User& user = users[addr.ip];
    user.sessionID = sessionID;
    user.name = data.pop<string>();
    user.timer.start();
    nextSessionID = user.sessionID + 1;
  }
  
  // detecting failures
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE) {
      users.erase(it++);
      if (state == STATE_IDLE)
        recoverFromFailure();
    }
    else
      ++it;
  }
}
