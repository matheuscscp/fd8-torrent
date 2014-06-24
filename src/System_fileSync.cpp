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
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_CREATE_FILE));
      conn.send(fullPath);
      conn.send(uint32_t(info.size()));
      conn.send(info);
    }
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
