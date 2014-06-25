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
#include "Helpers.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace network;
using namespace helpers;

void System::executeProtocol() {
  TCPConnection* peer = mainTCPServer.accept();
  if (peer == nullptr)
    return;
  
  char request = peer->recv<char>();
  switch (request) {
    case fd8protocol::MTYPE_SYNC:
      respondSystemState(peer);
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
      
    case fd8protocol::MTYPE_CREATE_FILE:
      {
        string fullPath = peer->recv<string>();
        ByteQueue info(peer->recv<uint32_t>());
        peer->recv(info);
        recv_createFile(fullPath, info);
      }
      break;
      
    case fd8protocol::MTYPE_UPDATE_FILE:
      {
        string fullPath = peer->recv<string>();
        recv_updateFile(fullPath, peer->recv<string>());
      }
      break;
      
    case fd8protocol::MTYPE_DELETE_FILE:
      recv_deleteFile(peer->recv<string>());
      break;
      
    default:
      break;
  }
  
  delete peer;
}
