/*
 * FileSystem.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

// this
#include "FileSystem.hpp"

using namespace std;
using namespace helpers;

map<string, FileSystem::Folder> FileSystem::folders;

void FileSystem::init() {
  folders.clear();
}

bool FileSystem::createFolder(const string& fullPath) {
  //TODO
  return false;
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  if (folders.find(fullPath) == folders.end())
    return nullptr;
  return &folders[fullPath];
}

bool FileSystem::updateFolder(const string& fullPath, const string& newPath) {
  //TODO
  return false;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  //TODO
  return false;
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {//FIXME
  int i;
  for (i = fullPath.size() - 1; i >= 0 && fullPath[i] != '/'; i--);
  return &folders[fullPath.substr(0, i)].files[fullPath.substr(i + 1, fullPath.size())];
}

int FileSystem::getTotalFiles() {
  int total = 0;
  for (auto& kv : folders)
    total += kv.second.files.size();
  return total;
}

int FileSystem::getTotalSize() {
  int total = 0;
  for (auto& kv1 : folders) {
    for (auto& kv2 : kv1.second.files)
      total += kv2.second.size;
  }
  return total;
}

ByteQueue FileSystem::readFile(FILE* fp) {
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  ByteQueue data(size);
  fread(data.ptr(), size, 1, fp);
  return data;
}
