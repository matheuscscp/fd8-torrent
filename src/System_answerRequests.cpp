/*
 * System_answerRequests.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Network.hpp"
#include "FD8Protocol.hpp"

using namespace network;

// static variables
static TCPConnection* peer;

void System::answerRequests() {
  peer = mainTCPServer.accept();
  if (peer == nullptr)
    return;
  
  char request = peer->recv<char>();
  switch (request) {
    case fd8protocol::MTYPE_GET_USERS:
      peer->send(uint32_t(users.size()));
      for (auto& kv : users) {
        peer->send(kv.first);
        peer->send(kv.second.name);
      }
      break;
      
    default:
      break;
  }
}
