/*
 * System_folderSync.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// local
#include "Network.hpp"
#include "Defines.hpp"
#include "FD8Protocol.hpp"
#include "FileSystem.hpp"

using namespace std;
using namespace concurrency;
using namespace network;
using namespace fd8protocol;

void System::send_createFolder(const string& fullPath) {
  Thread([&]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_CREATE_FOLDER));
      conn.send(fullPath);
    }
  }).start();
}

void System::send_updateFolder(const string& fullPath, const string& newName) {
  Thread([&]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_UPDATE_FOLDER));
      conn.send(fullPath);
      conn.send(newName);
    }
  }).start();
}

void System::send_deleteFolder(const string& fullPath) {
  Thread([&]() {
    for (auto& kv : users) {
      if (kv.first == localAddress.ip)
        continue;
      TCPConnection conn(Address(kv.first, Address("", TCPUDP_MAIN).port));
      conn.send(char(MTYPE_DELETE_FOLDER));
      conn.send(fullPath);
    }
  }).start();
}

void System::recv_createFolder(const string& fullPath) {
  FileSystem::createFolder(fullPath);
}

void System::recv_updateFolder(const string& fullPath, const string& newName) {
  FileSystem::updateFolder(fullPath, newName);
}

void System::recv_deleteFolder(const string& fullPath) {
  FileSystem::deleteFolder(fullPath);
}
