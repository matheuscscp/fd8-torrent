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
using namespace concurrency;

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
      
    case fd8protocol::MTYPE_COMMANDS:
      {
        ByteQueue data(peer->recv<uint32_t>());
        peer->recv(data);
        list<FileSystem::Command*> cmds = FileSystem::Command::deserialize(data);
        send_files(cmds);
        FileSystem::processCommands(cmds);
        for (auto& cmd : cmds)
          delete cmd;
      }
      break;
      
    case fd8protocol::MTYPE_FILE:
      {
        TCPConnection* tmpConn = peer;
        peer = nullptr;
        Thread([tmpConn]() {
          char tmp[25];
          uint32_t fileID = tmpConn->recv<uint32_t>();
          uint32_t fileSize = tmpConn->recv<uint32_t>();
          sprintf(tmp, "www/files/%08x", fileID);
          FILE* fp = fopen(tmp, "wb");
          size_t bytesRecvd = 0;
          ByteQueue buf;
          while (bytesRecvd < fileSize) {
            size_t diff = fileSize - bytesRecvd;
            buf.resize(SIZE_FILEBUFFER_MAXLEN < diff ? SIZE_FILEBUFFER_MAXLEN : diff);
            tmpConn->recv(buf);
            bytesRecvd += buf.size();
            fwrite(buf.ptr(), buf.size(), 1, fp);
          }
          fclose(fp);
          delete tmpConn;
        }).start();
      }
      break;

    default:
      break;
  }
  
  if(peer)
    delete peer;
}
