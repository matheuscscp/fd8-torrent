/*
 * FileSystem.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

// this
#include "FileSystem.hpp"

using namespace std;

map<string, FileSystem::Folder> FileSystem::folders;

void FileSystem::init() {
  folders.clear();
}

FileSystem::Folder& FileSystem::getFolder(const string& fullPath) {
  return folders[fullPath];
}

FileSystem::File& FileSystem::getFile(const string& fullPath) {
  int i;
  for (i = fullPath.size() - 1; i >= 0 && fullPath[i] != '/'; i--);
  return folders[fullPath.substr(0, i)].files[fullPath.substr(i + 1, fullPath.size())];
}
