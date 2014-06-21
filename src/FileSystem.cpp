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

bool FileSystem::parseFolderPath(const string& fullPath) {
  return false;//TODO
}

bool FileSystem::parseFilePath(const string& fullPath) {
  return false;//TODO
}

bool FileSystem::createFolder(const string& fullPath) {
  if (!parseFolderPath(fullPath)) // if the path is invalid
    return false;
  if (folders.find(fullPath) != folders.end()) // if the folder already exists
    return false;
  pair<string, string> divided = divide(fullPath, '/');
  auto motherFolder = folders.find(divided.first);
  if (motherFolder == folders.end()) // if the mother folder doesn't exist
    return false;
  motherFolder->second.subfolders.insert(fullPath);
  folders[fullPath];
  return true;
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  auto folder = folders.find(fullPath);
  if (folder == folders.end())
    return nullptr;
  return &folder->second;
}

bool FileSystem::updateFolder(const string& fullPath, const string& newPath) {
  //TODO
  return false;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  //TODO
  return false;
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {
  int i;
  for (i = fullPath.size() - 1; i >= 0 && fullPath[i] != '/'; i--);
  auto folder = folders.find(fullPath.substr(0, i));
  if (folder == folders.end())
    return nullptr;
  auto& files = folder->second.files;
  auto file = files.find(fullPath.substr(i + 1, fullPath.size()));
  if (file == folder->second.files.end())
    return nullptr;
  return &file->second;
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
