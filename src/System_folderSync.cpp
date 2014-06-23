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

using namespace std;
using namespace concurrency;
using namespace network;

void System::createFolder(const string& fullPath) {
  Thread([]() {
    for (auto& kv : users) {
      
    }
  }).start();
}

void System::updateFolder(const string& fullPath, const string& newName) {
  
}

void System::deleteFolder(const string& fullPath) {
  
}
