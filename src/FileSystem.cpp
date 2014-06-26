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

using namespace std;
using namespace helpers;

FileSystem::Folder FileSystem::rootFolder;
uint32_t FileSystem::nextID;
uint32_t FileSystem::localIP;
set<uint32_t> FileSystem::storedFiles;

FileSystem::File::File() : id(nextID++), size(0), peer1(localIP), peer2(0) {
  char tmp[25];
  sprintf(tmp, "www/files/%08x", id);
  rename("www/files/tmp", tmp);
  FILE* fp = fopen(tmp, "rb");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);
  }
}

void FileSystem::File::serialize(ByteQueue& data) {
  data.push((const void*)this, 16).push(author);
}

void FileSystem::File::deserialize(ByteQueue& data) {
  data.pop((void*)this, 16);
  author = data.pop<string>();
}

void FileSystem::File::erase() {
  char tmp[25];
  sprintf(tmp, "www/files/%08x", id);
  remove(tmp);
}

uint32_t FileSystem::Folder::getTotalFolders() {
  uint32_t total = subfolders.size();
  for (auto& kv : subfolders)
    total += kv.second.getTotalFolders();
  return total;
}

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

FileSystem::Folder* FileSystem::Folder::findFirstBottomUp(const string& subPath, string& foundPath) {
  if (!parsePath(subPath)) { // if the path is invalid
    foundPath = "";
    return nullptr;
  }
  Folder* folder = findFirstBottomUp_(subPath, foundPath);
  // removing duplicated '/'
  if (foundPath.size() > 2 && foundPath[0] == '/' && foundPath[1] == '/')
    foundPath = foundPath.substr(1, foundPath.size());
  return folder;
}

void FileSystem::Folder::serialize(ByteQueue& data) {
  data.push(uint32_t(subfolders.size()));
  for (auto& kv : subfolders) {
    data.push(kv.first);
    kv.second.serialize(data);
  }
  data.push(uint32_t(files.size()));
  for (auto& kv : files) {
    data.push(kv.first);
    kv.second.serialize(data);
  }
}

void FileSystem::Folder::deserialize(ByteQueue& data) {
  uint32_t subf_size = data.pop<uint32_t>();
  for (uint32_t i = 0; i < subf_size; i++) {
    string key = data.pop<string>();
    subfolders[key].deserialize(data);
  }
  uint32_t files_size = data.pop<uint32_t>();
  for (uint32_t i = 0; i < files_size; i++) {
    string key = data.pop<string>();
    files[key].deserialize(data);
  }
}

void FileSystem::Folder::eraseFiles() {
  for (auto& kv : subfolders)
    kv.second.eraseFiles();
  for (auto& kv : files)
    kv.second.erase();
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
  // recursive call
  return parentFolder->second.findFolder_(brokenPath.second, parent);
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
  // recursive call
  return parentFolder->second.findFile_(brokenPath.second, parent);
}

FileSystem::Folder* FileSystem::Folder::findFirstBottomUp_(const string& subPath, string& foundPath) {
  pair<string, string> brokenPath = extractFirst(subPath, '/');
  if (brokenPath.second == "") { // if subPath is a folder name
    auto folder = subfolders.find(brokenPath.first);
    if (folder == subfolders.end()) // if the folder was not found
      return this;
    foundPath += brokenPath.first;
    return &folder->second;
  }
  auto parentFolder = subfolders.find(brokenPath.first);
  if (parentFolder == subfolders.end()) // if a parent folder was not found
    return this;
  foundPath += brokenPath.first;
  // recursive call
  return parentFolder->second.findFirstBottomUp_(brokenPath.second, foundPath);
}

void FileSystem::init(uint32_t localIP) {
  rootFolder.subfolders.clear();
  rootFolder.files.clear();
  FileSystem::localIP = localIP;
  nextID = 1;
  storedFiles.clear();
#ifdef _WIN32
  system("rmdir /Q /S www\\files");
  system("mkdir www\\files");
#else
  system("rm -rf www/files");
  system("mkdir www/files");
#endif
}

ByteQueue FileSystem::serialize() {
  ByteQueue data;
  rootFolder.serialize(data);
  return data;
}

void FileSystem::deserialize(ByteQueue& data) {
  rootFolder.deserialize(data);
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
    if (allowedChars.find(name[i]) == allowedChars.end())
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

FileSystem::Folder* FileSystem::retrieveFolder(const string& fullPath, string& foundPath) {
  foundPath = "/";
  if (fullPath == "/") // returning root folder
    return &rootFolder;
  return rootFolder.findFirstBottomUp(fullPath, foundPath);
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
  return newFolder;
}

bool FileSystem::deleteFolder(const string& fullPath) {
  Folder* parent;
  Folder* folder = rootFolder.findFolder(fullPath, &parent);
  if (!parent || !folder) // if parent of folder doesn't exist
    return false;
  folder->eraseFiles();
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  parent->subfolders.erase(brokenPath.second);
  return true;
}

FileSystem::File* FileSystem::createFile(const string& fullPath, const string& author) {
  Folder* parent;
  File* file = rootFolder.findFile(fullPath, &parent);
  if (!parent || file) { // if parent folder was not found or the file exist
    remove("www/files/tmp");
    return nullptr;
  }
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  file = &parent->files[brokenPath.second];
  file->author = author;
  storedFiles.insert(file->id);
  return file;
}

FileSystem::File* FileSystem::createFile(const string& fullPath, ByteQueue& info) {
  Folder* parent;
  File* file = rootFolder.findFile(fullPath, &parent);
  if (!parent || file) // if parent folder was not found or the file exist
    return nullptr;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  file = &parent->files[brokenPath.second];
  file->deserialize(info);
  return file;
}

FileSystem::File* FileSystem::retrieveFile(const string& fullPath) {
  return rootFolder.findFile(fullPath);
}

FileSystem::File* FileSystem::updateFile(const string& fullPath, const string& newName) {
  if (!parseName(newName)) // if the new name is invalid
    return nullptr;
  Folder* parent;
  File* file = rootFolder.findFile(fullPath, &parent);
  if (!parent || !file) // if the parent folder or the file don't exist
    return nullptr;
  string newNameWithSlash = string("/") + newName;
  // if the new file already exist
  if (parent->files.find(newNameWithSlash) != parent->files.end())
    return nullptr;
  File* newFile = &parent->files[newNameWithSlash];
  *newFile = *file;
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  parent->files.erase(brokenPath.second);
  return newFile;
}

bool FileSystem::deleteFile(const string& fullPath) {
  Folder* parent;
  File* file = rootFolder.findFile(fullPath, &parent);
  if (!parent || !file) // if the parent folder or the file don't exist
    return false;
  file->erase();
  pair<string, string> brokenPath = extractLast(fullPath, '/');
  parent->files.erase(brokenPath.second);
  return true;
}

uint32_t FileSystem::getTotalFolders() {
  return rootFolder.getTotalFolders();
}

uint32_t FileSystem::getTotalFiles() {
  return rootFolder.getTotalFiles();
}

uint64_t FileSystem::getTotalSize() {
  return rootFolder.getTotalSize();
}
