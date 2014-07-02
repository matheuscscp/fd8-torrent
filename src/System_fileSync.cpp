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
      conn.recv<char>();
    }
    
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
      conn.recv<char>();
    }
    
    send_files(cmds);
    FileSystem::processCommands(cmds);
    
    for (auto& cmd : cmds)
      delete cmd;
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
      conn.recv<char>();
    }
  }).start();
}

void System::send_deleteFile(const string& fullPath) {
  Thread([this, fullPath]() {
    set<uint32_t> peers;
    
    for (auto& kv : users) {
      peers.insert(kv.first);
      
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_DELETE_FILE));
      conn.send(fullPath);
      conn.recv<char>();
    }
    
    FileSystem::initTmpFileSystem();
    list<FileSystem::Command*> cmds = FileSystem::calculateBalancing(peers);
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

void System::send_files(const list<FileSystem::Command*>& cmds) {
  for (auto& cmd : cmds) {
    if(cmd->type() == MTYPE_CMD_DUPLICATION && ((FileSystem::DuplicationCommand*)cmd)->srcPeer == localAddress.ip) {
      FileSystem::DuplicationCommand dupCmd = *((FileSystem::DuplicationCommand*)cmd);
      Thread([this, dupCmd]() {
        char buf[SIZE_FILEBUFFER_MAXLEN];
        
        // getting file name
        char tmp[25];
        sprintf(tmp, "www/files/%08x", dupCmd.fileID);
        
        FILE* fp = fopen(tmp, "rb");
        fseek(fp, 0, SEEK_END);
        
        TCPConnection conn(Address(dupCmd.dstPeer, Address("", TCPUDP_MAIN).port));
        conn.send(char(MTYPE_FILE));
        conn.send(uint32_t(dupCmd.fileID));
        conn.send(uint32_t(ftell(fp)));
        fclose(fp);
        fp = fopen(tmp, "rb");
        for (
          size_t readBytes;
          (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
          conn.send(buf, readBytes)
        );
        fclose(fp);
        conn.recv<char>();
      }).start();
    }
    else if (cmd->type() == MTYPE_CMD_BALANCING) {
      FileSystem::BalancingCommand balCmd = *((FileSystem::BalancingCommand*)cmd);
      string zuera;
      FileSystem::File file = *FileSystem::retrieveFolder("/", zuera)->findFile(balCmd.fileID);
      if (balCmd.srcPeer == localAddress.ip) {
        Thread([this, balCmd, file]() {
          char buf[SIZE_FILEBUFFER_MAXLEN];
          
          // getting file name
          char tmp[25];
          sprintf(tmp, "www/files/%08x", balCmd.fileID);
          
          // getting file size
          FILE* fp = fopen(tmp, "rb");
          fseek(fp, 0, SEEK_END);
          uint32_t fileSize = ftell(fp);
          fclose(fp);
          
          // sending to peer 1
          if (balCmd.peer1 != file.peer1 && balCmd.peer1 != file.peer2) {
            TCPConnection conn(Address(balCmd.peer1, Address("", TCPUDP_MAIN).port));
            conn.send(char(MTYPE_FILE));
            conn.send(uint32_t(balCmd.fileID));
            conn.send(fileSize);
            fp = fopen(tmp, "rb");
            for (
              size_t readBytes;
              (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
              conn.send(buf, readBytes)
            );
            fclose(fp);
            conn.recv<char>();
          }
          
          // sending to peer 2
          if (balCmd.peer2 != file.peer1 && balCmd.peer2 != file.peer2) {
            TCPConnection conn(Address(balCmd.peer2, Address("", TCPUDP_MAIN).port));
            conn.send(char(MTYPE_FILE));
            conn.send(uint32_t(balCmd.fileID));
            conn.send(fileSize);
            fp = fopen(tmp, "rb");
            for (
              size_t readBytes;
              (readBytes = fread(buf, 1, SIZE_FILEBUFFER_MAXLEN, fp)) > 0 && state == STATE_IDLE;
              conn.send(buf, readBytes)
            );
            fclose(fp);
            conn.recv<char>();
          }
          
          remove(tmp);
        }).start();
      }
      else if ((file.peer1 == localAddress.ip || file.peer2 == localAddress.ip) && balCmd.peer1 != localAddress.ip && balCmd.peer2 != localAddress.ip) {
        char tmp[25];
        sprintf(tmp, "www/files/%08x", balCmd.fileID);
        remove(tmp);
      }
    }
  }
}
