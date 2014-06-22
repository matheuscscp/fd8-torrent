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

FileSystem::Folder::Folder(Folder* parent) : parent(parent) {
  
}

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
    rootFolder.subfolders[fullPath].parent = rootFolder;
  }
  else { // if fullPath has two or more folders
    Folder* parentFolder = rootFolder.findFolder(brokenPath.first);
    if (!parentFolder) // if the parent folder doesn't exist
      return false;
    if (parentFolder->findFolder(brokenPath.second)) // if folder already exist
      return false;
    parentFolder->subfolders[brokenPath.second].parent = parentFolder;
  }
  return true;
}

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath) {
  if (fullPath == "/")
    return &rootFolder;
  return rootFolder.findFolder(fullPath);
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
