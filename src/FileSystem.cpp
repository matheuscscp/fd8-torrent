/*
 * FileSystem.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

// this
#include "FileSystem.hpp"

// standard
#include <cstdlib>
#include <set>

using namespace std;
using namespace helpers;

FileSystem::Folder FileSystem::rootFolder;
uint32_t FileSystem::nextID;
uint32_t FileSystem::localIP;

uint32_t FileSystem::Folder::getTotalFiles() {
  uint32_t total = 0;
  for (auto& kv : subfolders)
    total += kv.second.getTotalFiles();
  return total + files.size();
}

uint64_t FileSystem::Folder::getTotalSize() {
  int total = 0;
  for (auto& kv : subfolders)
    total += kv.second.getTotalSize();
  for (auto& kv : files)
    total += kv.second.size;
  return total;
}

FileSystem::Folder* FileSystem::Folder::findFolder(const string& subPath, Folder** parent) {
  if (!parsePath(subPath)) { // if the path is invalid
    if (parent) // if the parent folder was requested
      *parent = nullptr;
    return nullptr;
  }
  return findFolder_(subPath, parent);
}

FileSystem::File* FileSystem::Folder::findFile(const string& subPath, Folder** parent) {
  if (!parsePath(subPath)) { // if the path is invalid
    if (parent) // if the parent folder was requested
      *parent = nullptr;
    return nullptr;
  }
  return findFile_(subPath, parent);
}

FileSystem::Folder* FileSystem::Folder::findFolder_(const string& subPath, Folder** parent) {
  pair<string, string> brokenPath = extractFirst(subPath, '/');
  if (brokenPath.second == "") { // if subPath is a folder name
    if (parent) // if the parent folder was requested
      *parent = this;
    auto folder = subfolders.find(brokenPath.first);
    if (folder == subfolders.end()) // if the folder was not found
      return nullptr;
    return &folder->second;
  }
  auto parentFolder = subfolders.find(brokenPath.first);
  if (parentFolder == subfolders.end()) { // if a parent folder was not found
    if (parent) // if the parent folder was requested
      *parent = nullptr;
    return nullptr;
  }
  return parentFolder->second.findFolder_(brokenPath.second, parent); // recursive call
}

FileSystem::File* FileSystem::Folder::findFile_(const string& subPath, Folder** parent) {
  pair<string, string> brokenPath = extractFirst(subPath, '/');
  if (brokenPath.second == "") { // if supPath is a file name
    if (parent) // if the parent folder was requested
      *parent = this;
    auto file = files.find(brokenPath.first);
    if (file == files.end()) // if the file was not found
      return nullptr;
    return &file->second;
  }
  auto parentFolder = subfolders.find(brokenPath.first);
  if (parentFolder == subfolders.end()) { // if a parent folder was not found
    if (parent) // if the parent folder was requested
      *parent = nullptr;
    return nullptr;
  }
  return parentFolder->second.findFile_(brokenPath.second, parent); // recursive call
}

void FileSystem::init(uint32_t localIP) {
  rootFolder.subfolders.clear();
  rootFolder.files.clear();
  system("rm -rf www/files/*");
  FileSystem::localIP = localIP;
  nextID = 0;
}

bool FileSystem::parseName(const string& name) {
  static set<char> allowedChars;
  static StaticInitializer staticInitializar([&]() {
    for (char c = '0'; c <= '9'; c++)
      allowedChars.insert(c);
    for (char c = 'a'; c <= 'z'; c++)
      allowedChars.insert(c);
    for (char c = 'A'; c <= 'Z'; c++)
      allowedChars.insert(c);
    allowedChars.insert('_');
    allowedChars.insert('-');
    allowedChars.insert('+');
    allowedChars.insert('.');
  });
  if (!name.size()) // if the name is empty
    return false;
  for (int i = 0; i < int(name.size()); i++) { // check if all chars are allowed
    if (allowedChars.find(name[i]) != allowedChars.end())
      return false;
  }
  return true;
}

bool FileSystem::parsePath(const string& path) {
  if (!path.size()) // if the path is empty
    return false;
  list<string> atoms = explode(path, '/');
  string reassembledPath;
  for (auto& folder : atoms) { // reassemble the exploded path
    if (!parseName(folder)) // if there is an invalid folder name
      return false;
    reassembledPath += '/';
    reassembledPath += folder;
  }
  return reassembledPath == path;
}

FileSystem::Folder* FileSystem::createFolder(const string& fullPath) {
  Folder* parent;
  Folder* folder = rootFolder.findFolder(fullPath, &parent);
  if (!parent || folder) // if parent folder was not found or the folder exist
    return nullptr;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  return &parent->subfolders[brokenPath.second];
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  if (fullPath == "/")
    return &rootFolder;
  return rootFolder.findFolder(fullPath);
}

FileSystem::Folder* FileSystem::updateFolder(const string& fullPath, const string& newName) {
  if (!parseName(newName)) // if the new name is invalid
    return nullptr;
  Folder* parent;
  Folder* folder = rootFolder.findFolder(fullPath, &parent);
  if (!parent || !folder) // if parent or folder doesn't exist
    return nullptr;
  string newNameWithSlash = string("/") + newName;
  // if the new folder already exist
  if (parent->subfolders.find(newNameWithSlash) != parent->subfolders.end())
    return nullptr;
  Folder* newFolder = &parent->subfolders[newNameWithSlash];
  newFolder->subfolders = folder->subfolders;
  newFolder->files = folder->files;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  parent->subfolders.erase(brokenPath.second);
  //TODO rename files in this peer
  return newFolder;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  Folder* parent;
  Folder* folder = rootFolder.findFolder(fullPath, &parent);
  if (!parent || !folder) // if parent of folder doesn't exist
    return false;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  parent->subfolders.erase(brokenPath.second);
  //TODO remove files in this peer
  return false;
}

FileSystem::File* FileSystem::createFile(const string& fullPath, const ByteQueue& byteQueue) {
  return nullptr;//TODO
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {
  return rootFolder.findFile(fullPath);
}

FileSystem::File* FileSystem::updateFile(const string& fullPath, const string& newName) {
  return nullptr;//TODO
}

bool FileSystem::deleteFile(const string& fullPath) {
  return false;//TODO
}

uint32_t FileSystem::getTotalFiles() {
  return rootFolder.getTotalFiles();
}

uint64_t FileSystem::getTotalSize() {
  return rootFolder.getTotalSize();
}

ByteQueue FileSystem::readFile(FILE* fp) {
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  ByteQueue data(size);
  fread(data.ptr(), size, 1, fp);
  return data;
}
