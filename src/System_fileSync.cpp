/*
 * System_fileSync.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Defines.hpp"
#include "FD8Protocol.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace helpers;
using namespace concurrency;
using namespace network;
using namespace fd8protocol;

void System::send_createFile(const string& fullPath, const ByteQueue& info) {
  Thread([this, fullPath, info]() {
    set<uint32_t> peers;
    
    for (auto& kv : users) {
      peers.insert(kv.first);
      
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_CREATE_FILE));
      conn.send(fullPath);
      conn.send(uint32_t(info.size()));
      conn.send(info);
    }
    
    list<FileSystem::DuplicationCommand> cmds = FileSystem::calculateDuplications(peers);
    for (auto& kv : users){
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
  }).start();
}

void System::send_updateFile(const string& fullPath, const string& newName) {
  Thread([this, fullPath, newName]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_UPDATE_FILE));
      conn.send(fullPath);
      conn.send(newName);
    }
  }).start();
}

void System::send_deleteFile(const string& fullPath) {
  Thread([this, fullPath]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_DELETE_FILE));
      conn.send(fullPath);
    }
  }).start();
}

void System::recv_createFile(const string& fullPath, ByteQueue& info) {
  FileSystem::createFile(fullPath, info);
}

void System::recv_updateFile(const string& fullPath, const string& newName) {
  FileSystem::updateFile(fullPath, newName);
}

void System::recv_deleteFile(const string& fullPath) {
  FileSystem::deleteFile(fullPath);
}

void System::send_fileDuplications(const list<FileSystem::DuplicationCommand>& cmds) {
  for(auto& cmd : cmds){
    if(cmd.srcPeer == localAddress.ip){
      Thread([this, cmd]() {
        char tmp[25];
        char buf[SIZE_FILEBUFFER_MAXLEN];
        TCPConnection conn(Address(cmd.dstPeer, Address("", TCPUDP_MAIN).port));
        sprintf(tmp, "www/files/%08x", cmd.fileID);
        FILE* fp = fopen(tmp, "rb");
        fseek(fp, 0, SEEK_END);
        conn.send(char(fd8protocol::MTYPE_FILE));
        conn.send(uint32_t(cmd.fileID));
        conn.send(uint32_t(ftell(fp)));
        fclose(fp);
        fp = fopen(tmp, "rb");
        for (
          size_t readBytes;
          (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
          conn.send(buf, readBytes)
        );
        fclose(fp);
      }).start();
    }
  }
}
