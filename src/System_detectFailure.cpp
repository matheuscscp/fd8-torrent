/*
 * System_recoverFromFailure.cpp
 *
 *  Created on: Jun 29, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "FD8Protocol.hpp"

using namespace std;
using namespace helpers;
using namespace network;
using namespace fd8protocol;

void System::detectFailure() {
  bool failureDetected = false;
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE) {
      users.erase(it++);
      failureDetected = true;
    }
    else
      ++it;
  }
  
  if (failureDetected && state == STATE_IDLE)
    recoverFromFailure();
}

void System::recoverFromFailure() {
  // check who is the designated peer
  set<uint32_t> peers;
  uint32_t designatedPeer = 0;
  for (auto& kv : users) {
    peers.insert(kv.first);
    if (kv.first > designatedPeer)
      designatedPeer = kv.first;
  }
  
  // if the designated peer is not this one
  if (designatedPeer != localAddress.ip)
    return;
  
  // recover
  FileSystem::initTmpFileSystem();
  list<FileSystem::Command*> cmds = FileSystem::calculateDuplications(peers);
  list<FileSystem::Command*> balCmds = FileSystem::calculateBalancing(peers);
  for (auto& cmd : balCmds)
    cmds.push_back(cmd);
  FileSystem::eliminateIntersections(cmds);
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
