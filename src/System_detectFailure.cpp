/*
 * System_detectFailure.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "FD8Protocol.hpp"

using namespace std;
using namespace network;
using namespace fd8protocol;

void System::detectFailure() {
  set<uint32_t> peers;
  for (auto it = users.begin(); it != users.end();) {
    if (it->second.timer.time() >= MS_DETECTFAILURE) {
      users.erase(it++);
    }
    else {
      peers.insert(it->first);
      ++it;
    }
  }
  
  if (state == STATE_LOGIN)
    return;
  
  uint32_t designatedPeer = 0;
  for (auto& kv : users) {
    if (kv.first > designatedPeer)
      designatedPeer = kv.first;
  }
  if (designatedPeer == localAddress.ip) {
    list<FileSystem::DuplicationCommand> cmds = FileSystem::calculateDuplications(peers);
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_DUPLICATION));
      conn.send(uint32_t(cmds.size()));
      for (auto& cmd : cmds){
        conn.send(&cmd, 12);
      }
    }
    send_fileDuplications(cmds);
  }
}
