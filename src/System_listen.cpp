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
  
  // listening only to non-empty messages, with valid IP address from other peer
  if (!data.size() || addr.ip == localAddress.ip || !addr.isPrivateNetwork())
    return;
  
  uint32_t sessionID = data.pop<uint32_t>();
  auto userIt = users.find(addr.ip);
  if (userIt != users.end()) { // IP already in the user table
    if (userIt->second.sessionID == sessionID) // refresh beacon
      userIt->second.timer.start();
    else { // someone logged out and then logged in
      users.erase(userIt); // TODO MUNDO TEM QUE DETECTAR A FALHA PRA COMECAR A ZUERA
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
        conn.recv<char>();
      }
      send_files(cmds);
      FileSystem::processCommands(cmds);
      for (auto& cmd : cmds)
        delete cmd;
    }
  }
}
