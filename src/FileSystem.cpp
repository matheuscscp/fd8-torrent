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

FileSystem::Folder FileSystem::rootFolder(nullptr);
uint32_t FileSystem::localIP;

void FileSystem::Folder::clear() {
  subfolders.clear();
  files.clear();
}

int FileSystem::Folder::getTotalFiles() {
  int total = 0;
  for (auto& kv : subfolders)
    total += kv.second.getTotalFiles();
  return total + files.size();
}

int FileSystem::Folder::getTotalSize() {
  int total = 0;
  for (auto& kv : subfolders)
    total += kv.second.getTotalSize();
  for (auto& kv : files)
    total += kv.second.size;
  return total;
}

FileSystem::Folder* FileSystem::Folder::findFolder(const string& subPath) {
  if (!parsePath(subPath)) // if the path is invalid
    return nullptr;
  pair<string, string> brokenPath = extractFirst(subPath, '/');
  auto folder = subfolders.find(brokenPath.first);
  if (folder == subfolders.end()) // if the first name is not in subfolders
    return nullptr;
  if (brokenPath.second == "") // if first name is the only name
    return &folder->second;
  return folder->second.findFolder(brokenPath.second); // recursive call
}

FileSystem::File* FileSystem::Folder::findFile(const string& subPath) {
  if (!parsePath(subPath)) // if the path is invalid
    return nullptr;
  pair<string, string> brokenPath = extractFirst(subPath, '/');
  if (brokenPath.second == "") { // if supPath is a file name
    auto file = files.find(brokenPath.first);
    if (file == files.end()) // if the file was not found
      return nullptr;
    return &file->second;
  }
  auto folder = subfolders.find(brokenPath.first);
  if (folder == subfolders.end()) // if folder was not found
    return nullptr;
  return folder->second.findFile(brokenPath.second); // recursive call
}

void FileSystem::init(uint32_t localIP) {
  rootFolder.clear();
  system("rm -rf www/files/*");
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
  string reassembledPath;
  for (auto& folder : atoms) { // reassemble the exploded path
    if (!parseName(folder)) // if there is an invalid folder name
      return false;
    reassembledPath += '/';
    reassembledPath += folder;
  }
  return reassembledPath == path;
}

bool FileSystem::createFolder(const string& fullPath) {
  if (!parsePath(fullPath)) // if the path is invalid
    return false;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  if (brokenPath.second == "") { // if fullPath is a folder itself
    if (rootFolder.findFolder(fullPath)) // if the folder already exist
      return false;
    rootFolder.subfolders[fullPath];
  }
  else { // if fullPath has two or more folders
    Folder* parentFolder = rootFolder.findFolder(brokenPath.first);
    if (!parentFolder) // if the parent folder doesn't exist
      return false;
    if (parentFolder->findFolder(brokenPath.second)) // if folder already exist
      return false;
    parentFolder->subfolders[brokenPath.second];
  }
  return true;
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  if (fullPath == "/")
    return &rootFolder;
  return rootFolder.findFolder(fullPath);
}

bool FileSystem::updateFolder(const string& fullPath, const string& newName) {
  if (!parsePath(fullPath) || !parseName(newName)) // if args are invalid
    return false;
  Folder* folder = retrieveFolder(fullPath);
  if (!folder) // if the folder doesn't exist
    return false;
  Folder* newFolder;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  if (brokenPath.second == "") // if fullPath is a folder itself
    newFolder = createFolder(string("/") + newName);
  else // if fullPath has two or more names
    newFolder = createFolder((brokenPath.first + "/") + newName);
  if (!newFolder) // if a folder already exist with the new path
    return false;
  newFolder->subfolders = folder->subfolders;
  newFolder->files = folder->files;
  //TODO rename files in this peer
  return true;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  //TODO
  return false;
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {
  return rootFolder.findFile(fullPath);
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
