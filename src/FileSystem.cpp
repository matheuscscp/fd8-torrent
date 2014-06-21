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

FileSystem::Folder FileSystem::rootFolder;
uint32_t FileSystem::localIP;

int FileSystem::Folder::getTotalFiles() {
  int total = files.size();
  for (auto& kv : subfolders)
    total += kv.second.getTotalFiles();
  return total;
}

int FileSystem::Folder::getTotalSize() {
  int total = 0;
  for (auto& kv : files)
    total += kv.second.size;
  for (auto& kv : subfolders)
    total += kv.second.getTotalSize();
  return total;
}

void FileSystem::init(uint32_t localIP) {
  rootFolder.subfolders.clear();
  rootFolder.files.clear();
  system("rm www/files/*");
  FileSystem::localIP = localIP;
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
  if (!atoms.size()) // if no atom was found
    return false;
  auto it = atoms.begin();
  if (!parseName(*it)) // if the first name is invalid
    return false;
  string reassembledPath = *it;
  it++;
  for (int i = 1; i < int(atoms.size()); i++) { // if there's an invalid name
    if (!parseName(*it))
      return false;
    reassembledPath += '/';
    reassembledPath += (*it);
    it++;
  }
  return reassembledPath == path;
}

bool FileSystem::createFolder(const string& fullPath) {
  if (!parsePath(fullPath)) // if the path is invalid
    return false;
  if (folders.find(fullPath) != folders.end()) // if the folder already exists
    return false;
  pair<string, string> divided = divide(fullPath, '/');
  auto motherFolder = folders.find(divided.first);
  if (motherFolder == folders.end()) // if the mother folder doesn't exist
    return false;
  motherFolder->second.subfolders.insert(divided.second);
  folders[fullPath];
  return true;
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  auto folder = folders.find(fullPath);
  if (folder == folders.end())
    return nullptr;
  return &folder->second;
}

bool FileSystem::updateFolder(const string& fullPath, const string& newName) {
  if (fullPath == "root") // if the full path is the root folder
    return false;
  if (!parseName(newName)) // if the new name is invalid
    return false;
  pair<string, string> fullDivided = divide(fullPath, '/');
  if (fullDivided.second == newName) // if the new name is the current name
    return false;
  auto folder = folders.find(fullPath);
  if (folder == folders.end()) // if the folder doesn't exist
    return false;
  
  // create a new folder
  string newPath = (fullDivided.first + "/") + newName;
  auto& newFolder = folders[newPath];
  newFolder.subfolders = folder->second.subfolders;
  newFolder.files = folder->second.files;
  
  // erase old folder
  folders.erase(folder);
  
  for (auto& subfolder : newFolder.subfolders)
    updateFolder((fullPath + "/")
  
  // rename files stored in this peer
  //TODO
  
  return true;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  //TODO
  return false;
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {
  return nullptr;//TODO
}

int FileSystem::getTotalFiles() {
  return rootFolder.getTotalFiles();
}

int FileSystem::getTotalSize() {
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
