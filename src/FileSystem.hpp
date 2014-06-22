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
        int size;
        uint32_t peer1;
        uint32_t peer2;
        std::string author;
    };
    
    struct Folder {
      public:
        std::map<std::string, Folder> subfolders;
        std::map<std::string, File> files;
        void clear();
        int getTotalFiles();
        int getTotalSize();
        Folder* findFolder(const std::string& subPath);
        File* findFile(const std::string& subPath);
    };
  private:
    static Folder rootFolder;
    static uint32_t localIP;
  public:
    static void init(uint32_t localIP);
    
    static bool parseName(const std::string& name);
    static bool parsePath(const std::string& path);
    
    static bool createFolder(const std::string& fullPath);
    static Folder* retrieveFolder(const std::string& fullPath);
    static bool updateFolder(const std::string& fullPath, const std::string& newName);
    static bool deleteFolder(const std::string& fullPath);
    
    static File* retrieveFile(const std::string& fullPath);
    
    static int getTotalFiles();
    static int getTotalSize();
    
    static helpers::ByteQueue readFile(FILE* fp);
};

#endif /* FILESYSTEM_HPP_ */
