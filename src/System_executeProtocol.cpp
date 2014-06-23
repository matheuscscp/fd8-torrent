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

using namespace std;
using namespace network;

void System::executeProtocol() {
  TCPConnection* peer = mainTCPServer.accept();
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
      
    case fd8protocol::MTYPE_CREATE_FOLDER:
      recv_createFolder(peer->recv<string>());
      break;
      
    case fd8protocol::MTYPE_UPDATE_FOLDER:
      {
        string fullPath = peer->recv<string>();
        recv_updateFolder(fullPath, peer->recv<string>());
      }
      break;
      
    case fd8protocol::MTYPE_DELETE_FOLDER:
      recv_deleteFolder(peer->recv<string>());
      break;
      
    default:
      break;
  }
  
  delete peer;
}
