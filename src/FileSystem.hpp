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
#include <set>
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
        std::set<std::string> subfolders; // value: full path
        std::map<std::string, File> files; // key: only file name
    };
  private:
    static std::map<std::string, Folder> folders; // key: full path
  public:
    static void init();
    static Folder& getFolder(const std::string& fullPath);
    static File& getFile(const std::string& fullPath);
    static int getTotalFiles();
    static int getTotalSize();
    static helpers::ByteQueue readFile(FILE* fp);
};

#endif /* FILESYSTEM_HPP_ */
