/*
 * FileSystem.hpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

#ifndef FILESYSTEM_HPP_
#define FILESYSTEM_HPP_

// standard
#include <cstdint>
#include <string>
#include <map>
#include <cstdio>

// local
#include "Helpers.hpp"

class FileSystem {
  public:
    struct File {
      public:
        uint32_t id;
        uint32_t size;
        uint32_t peer1;
        uint32_t peer2;
        std::string author;
    };
    
    struct Folder {
      public:
        std::map<std::string, Folder> subfolders;
        std::map<std::string, File> files;
        uint32_t getTotalFiles();
        uint64_t getTotalSize();
        Folder* findFolder(const std::string& subPath, Folder** parent = nullptr);
        File* findFile(const std::string& subPath, Folder** parent = nullptr);
      private:
        Folder* findFolder_(const std::string& subPath, Folder** parent);
        File* findFile_(const std::string& subPath, Folder** parent);
    };
  private:
    static Folder rootFolder;
    static uint32_t nextID;
    static uint32_t localIP;
  public:
    static void init(uint32_t localIP);
    
    static bool parseName(const std::string& name);
    static bool parsePath(const std::string& path);
    
    static Folder* createFolder(const std::string& fullPath);
    static Folder* retrieveFolder(const std::string& fullPath);
    static Folder* updateFolder(const std::string& fullPath, const std::string& newName);
    static bool deleteFolder(const std::string& fullPath);
    
    static File* createFile(const std::string& fullPath, const helpers::ByteQueue& byteQueue);
    static File* retrieveFile(const std::string& fullPath);
    static File* updateFile(const std::string& fullPath, const std::string& newName);
    static bool deleteFile(const std::string& fullPath);
    
    static uint32_t getTotalFiles();
    static uint64_t getTotalSize();
    
    static helpers::ByteQueue readFile(FILE* fp);
};

#endif /* FILESYSTEM_HPP_ */
