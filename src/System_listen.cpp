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
  if (data.size() && addr.ip != localAddress.ip && Address::isPrivateNetwork(addr.ip)) {
    // update timeout
    uint32_t sessionID = data.pop<uint32_t>();
    auto userIt = users.find(addr.ip);
    if (userIt != users.end()) { // IP already in the user table
      if (userIt->second.sessionID == sessionID) // refresh beacon
        userIt->second.timer.start();
      else { // failure detected
        users.erase(userIt);
        if (state == STATE_IDLE)
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
      
      if (state == STATE_IDLE) {
        set<uint32_t> peers;
        for (auto& kv : users)
          peers.insert(kv.first);
        list<FileSystem::Command*> cmds;
        FileSystem::initTmpFileSystem();
        if (users.size() == 2)
          cmds = FileSystem::calculateDuplications(peers);
        else
          cmds = FileSystem::calculateBalancing(peers);
        ByteQueue data = FileSystem::Command::serialize(cmds);
        for (auto& kv : users) {
          if (kv.first == localAddress.ip)
            continue;
          TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
          conn.send(char(MTYPE_COMMANDS));
          conn.send(uint32_t(data.size()));
          conn.send(data);
        }
        send_files(cmds);
        FileSystem::processCommands(cmds);
        for (auto& cmd : cmds)
          delete cmd;
      }
    }
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
