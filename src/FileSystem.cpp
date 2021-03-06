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
#include <cstdio>
#include <cmath>

using namespace std;
using namespace helpers;

FileSystem::Folder FileSystem::rootFolder;
FileSystem::Folder FileSystem::tmpRootFolder;
uint32_t FileSystem::nextID;
uint32_t FileSystem::localIP;
set<uint32_t> FileSystem::storedFiles;

FileSystem::File::File() : id(nextID), size(0), peer1(localIP), peer2(0) {
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

void FileSystem::Folder::getPeersFiles(map<uint32_t, set<uint32_t>>& peersFiles) {
  for (auto& kv : subfolders)
    kv.second.getPeersFiles(peersFiles);
  for (auto& kv : files) {
    // checking if someone went offline
    if (peersFiles.find(kv.second.peer1) == peersFiles.end()) {
      kv.second.peer1 = kv.second.peer2;
      kv.second.peer2 = 0;
    }
    else if (peersFiles.find(kv.second.peer2) == peersFiles.end())
      kv.second.peer2 = 0;
    
    peersFiles[kv.second.peer1].insert(kv.second.id);
    if (kv.second.peer2)
      peersFiles[kv.second.peer2].insert(kv.second.id);
  }
}

void FileSystem::Folder::getSingleFiles(set<uint32_t>& singleFiles) {
  for (auto& kv : subfolders)
    kv.second.getSingleFiles(singleFiles);
  for (auto& kv : files) {
    if (!kv.second.peer2)
      singleFiles.insert(kv.second.id);
  }
}

void FileSystem::Folder::removeOfflinePeers(const set<uint32_t>& peers) {
  for (auto& kv : subfolders)
    kv.second.removeOfflinePeers(peers);
  for (auto& kv : files) {
    if (peers.find(kv.second.peer1) == peers.end()) {
      kv.second.peer1 = kv.second.peer2;
      kv.second.peer2 = 0;
    }
    else if (peers.find(kv.second.peer2) == peers.end())
      kv.second.peer2 = 0;
  }
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

FileSystem::File* FileSystem::Folder::findFile(uint32_t fileID){
  for(auto& kv : files){
    if(kv.second.id == fileID)
      return &kv.second;
  }
  for (auto& kv : subfolders) {
    File* file = kv.second.findFile(fileID);
    if (file)
      return file;
  }
  return nullptr;
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


ByteQueue FileSystem::Command::serialize(const list<Command*>& cmds) {
  ByteQueue data;
  for (auto& cmd : cmds)
    cmd->serialize(data);
  return data;
}

list<FileSystem::Command*> FileSystem::Command::deserialize(ByteQueue& data) {
  list<Command*> cmds;
  while (data.size()) {
    char mtype = data.pop<char>();
    switch (mtype) {
      case DUPLICATION:
        cmds.push_back(new DuplicationCommand(data));
        break;
        
      case BALANCING:
        cmds.push_back(new BalancingCommand(data));
        break;
        
      default:
        break;
    }
  }
  return cmds;
}

FileSystem::Command::~Command() {
  
}

void FileSystem::Command::serialize(ByteQueue& data) {
  data.push(type());
  serialize_(data);
}

FileSystem::DuplicationCommand::DuplicationCommand(ByteQueue& data) :
fileID(data.pop<uint32_t>()), srcPeer(data.pop<uint32_t>()), dstPeer(data.pop<uint32_t>()) {
  
}

FileSystem::DuplicationCommand::DuplicationCommand(uint32_t fileID, uint32_t srcPeer, uint32_t dstPeer) :
fileID(fileID), srcPeer(srcPeer), dstPeer(dstPeer) {
  
}

void FileSystem::DuplicationCommand::serialize_(ByteQueue& data) {
  data.push(fileID).push(srcPeer).push(dstPeer);
}

char FileSystem::DuplicationCommand::type() {
  return DUPLICATION;
}

FileSystem::BalancingCommand::BalancingCommand(ByteQueue& data) :
fileID(data.pop<uint32_t>()), srcPeer(data.pop<uint32_t>()), peer1(data.pop<uint32_t>()), peer2(data.pop<uint32_t>()) {
  
}

FileSystem::BalancingCommand::BalancingCommand(uint32_t fileID, uint32_t srcPeer, uint32_t peer1, uint32_t peer2) :
fileID(fileID), srcPeer(srcPeer), peer1(peer1), peer2(peer2) {
  
}

void FileSystem::BalancingCommand::serialize_(ByteQueue& data) {
  data.push(fileID).push(srcPeer).push(peer1).push(peer2);
}

char FileSystem::BalancingCommand::type() {
  return BALANCING;
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
  data.push(nextID);
  rootFolder.serialize(data);
  return data;
}

void FileSystem::deserialize(ByteQueue& data) {
  nextID = data.pop<uint32_t>();
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
  nextID++;
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
  nextID = file->id + 1;
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

list<FileSystem::Command*> FileSystem::calculateDuplications(const set<uint32_t>& peers) {
  list<Command*> cmds;
  
  if (peers.size() == 1)
    return cmds;
  
  // get files of each peer
  map<uint32_t, set<uint32_t>> peersFiles;
  for (auto& peer : peers)
    peersFiles[peer];
  tmpRootFolder.getPeersFiles(peersFiles);
  
  // get files to be duplicated
  set<uint32_t> singleFiles;
  tmpRootFolder.getSingleFiles(singleFiles);
  
  while (singleFiles.size()) {
    // find the peer with least files in peersFiles
    auto minimalPeer = peersFiles.begin();
    auto peerIt = peersFiles.begin();
    for (peerIt++; peerIt != peersFiles.end(); peerIt++) {
      if (peerIt->second.size() < minimalPeer->second.size())
        minimalPeer = peerIt;
    }
    
    // searching file to duplicate in the minimal peer
    auto fileToDup = singleFiles.begin();
    for (; fileToDup != singleFiles.end() && minimalPeer->second.find(*fileToDup) != minimalPeer->second.end(); fileToDup++);
    if (fileToDup == singleFiles.end()) // if the peer already have all the toDup files
      peersFiles.erase(minimalPeer);
    else { // file to duplicate found for this minimal peer
      uint32_t fileID = *fileToDup;
      
      // move from one set to the other
      singleFiles.erase(fileToDup);
      minimalPeer->second.insert(fileID);
      
      // update temporary file system
      File& file = *tmpRootFolder.findFile(fileID);
      file.peer2 = minimalPeer->first;
      
      // push command
      cmds.push_back(new DuplicationCommand(fileID, file.peer1, file.peer2));
    }
  }
  
  return cmds;
}

list<FileSystem::Command*> FileSystem::calculateBalancing(const set<uint32_t>& peers) {
  list<Command*> cmds;
  map<uint32_t, BalancingCommand*> tmpCmds;
  
  if (peers.size() <= 2)
    return cmds;
  
  // get files of each peer
  map<uint32_t, set<uint32_t>> peersFiles;
  for (auto& peer : peers)
    peersFiles[peer];
  tmpRootFolder.getPeersFiles(peersFiles);
  
  uint32_t averageFiles = uint32_t(ceilf(float(tmpRootFolder.getTotalFiles()*2)/peers.size()));
  
  while (true) { // one iteration of this loop moves a single file
    // find the peers with least and more files in peersFiles
    auto minimalPeer = peersFiles.begin(), maximalPeer = peersFiles.begin();
    auto peerIt = peersFiles.begin();
    for (peerIt++; peerIt != peersFiles.end(); peerIt++) {
      if (peerIt->second.size() < minimalPeer->second.size())
        minimalPeer = peerIt;
      else if (peerIt->second.size() > maximalPeer->second.size())
        maximalPeer = peerIt;
    }
    
    if (maximalPeer->second.size() <= averageFiles) // balanced
      break;
    
    for (auto& fileID : maximalPeer->second) { // choose a file to move
      // if this fileID is already being held by both minimal and maximal peers
      if (minimalPeer->second.find(fileID) != minimalPeer->second.end())
        continue;
      
      // file to move found
      
      // move from one set to the other
      maximalPeer->second.erase(fileID);
      minimalPeer->second.insert(fileID);
      
      // update temporary file system
      File& file = *tmpRootFolder.findFile(fileID);
      if (file.peer1 == maximalPeer->first)
        file.peer1 = minimalPeer->first;
      else
        file.peer2 = minimalPeer->first;
      
      auto balCmd = tmpCmds.find(fileID);
      if (balCmd == tmpCmds.end()) { // push command
        tmpCmds[fileID] = new BalancingCommand(fileID, maximalPeer->first, file.peer1, file.peer2);
      }
      else { // update command
        balCmd->second->peer1 = file.peer1;
        balCmd->second->peer2 = file.peer2;
      }
      
      break;
    }
  }
  
  for (auto& kv : tmpCmds)
    cmds.push_back(kv.second);
  
  return cmds;
}

void FileSystem::eliminateIntersections(list<Command*>& cmds) {
  // getting files to move
  map<uint32_t, BalancingCommand*> filesToMove;
  auto cmd = cmds.begin();
  for (; cmd != cmds.end() && (*cmd)->type() == Command::DUPLICATION; cmd++);
  for (; cmd != cmds.end(); cmds.erase(cmd++)) {
    BalancingCommand* balCmd = (BalancingCommand*)*cmd;
    filesToMove[balCmd->fileID] = balCmd;
  }
  
  // removing duplication commands that match a file to move
  for (cmd = cmds.begin(); cmd != cmds.end();) {
    DuplicationCommand& dupCmd = *((DuplicationCommand*)*cmd);
    auto balCmd = filesToMove.find(dupCmd.fileID);
    if (balCmd != filesToMove.end()) {
      balCmd->second->srcPeer = dupCmd.srcPeer;
      cmds.erase(cmd++);
    }
    else
      ++cmd;
  }
  
  // moving balacing commands back to the list
  for (auto& kv : filesToMove)
    cmds.push_back(kv.second);
}

void FileSystem::processCommands(const list<Command*>& cmds) {
  for (auto& cmd : cmds) {
    switch (cmd->type()) {
      case Command::DUPLICATION: {
        DuplicationCommand& dupCmd = *((DuplicationCommand*)cmd);
        File& file = *rootFolder.findFile(dupCmd.fileID);
        file.peer1 = dupCmd.srcPeer;
        file.peer2 = dupCmd.dstPeer;
        break;
      }
      
      case Command::BALANCING: {
        BalancingCommand& balCmd = *((BalancingCommand*)cmd);
        File& file = *rootFolder.findFile(balCmd.fileID);
        file.peer1 = balCmd.peer1;
        file.peer2 = balCmd.peer2;
        break;
      }
      
      default:
        break;
    }
  }
}

void FileSystem::initTmpFileSystem() {
  tmpRootFolder = rootFolder;
}
