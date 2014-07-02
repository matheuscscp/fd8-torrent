/*
 * System_stateIdle.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "FileSystem.hpp"
#include "FD8Protocol.hpp"

using namespace std;
using namespace network;
using namespace fd8protocol;
using namespace concurrency;
using namespace helpers;

void System::changeToIdle() {
  FileSystem::init(localAddress.ip);
  requestSystemState();
  idleBalancingTimer.start();
  httpThread = Thread([this]() {
    while (state == newState) {
      httpServer();
      Thread::sleep(MS_SLEEP);
    }
  });
  httpThread.start();
}

void System::stateIdle() {
  // first balancing of this peer
  if (idleBalancingTimer.time() > MS_DETECTFAILURE) {
    idleBalancingTimer.reset();
    
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
  
  executeProtocol();
  speak();
  listen();
  detectFailure();
}
