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
#include <set>
#include <cstdio>
#include <list>
#include <set>

// local
#include "Helpers.hpp"

class FileSystem {
  public:
    class File {
      public:
        uint32_t id;
        uint32_t size;
        uint32_t peer1;
        uint32_t peer2;
        std::string author;
        
        File();
        
        void serialize(helpers::ByteQueue& data);
        void deserialize(helpers::ByteQueue& data);
        
        void erase();
    };
    
    class Folder {
      public:
        std::map<std::string, Folder> subfolders;
        std::map<std::string, File> files;
        
        uint32_t getTotalFolders();
        uint32_t getTotalFiles();
        uint64_t getTotalSize();
        
        void getPeersFiles(std::map<uint32_t, std::set<uint32_t>>& peersFiles);
        void getFilesPeers(std::map<uint32_t, std::pair<uint32_t, uint32_t>>& filesPeers);
        
        Folder* findFolder(const std::string& subPath, Folder** parent = nullptr);
        File* findFile(const std::string& subPath, Folder** parent = nullptr);
        Folder* findFirstBottomUp(const std::string& subPath, std::string& foundPath);
        File* findFile(uint32_t fileID);
        
        void serialize(helpers::ByteQueue& data);
        void deserialize(helpers::ByteQueue& data);
        
        void eraseFiles();
      private:
        Folder* findFolder_(const std::string& subPath, Folder** parent);
        File* findFile_(const std::string& subPath, Folder** parent);
        Folder* findFirstBottomUp_(const std::string& subPath, std::string& foundPath);
    };
    
    class Command {
      public:
        static helpers::ByteQueue serialize(const std::list<Command*>& cmds);
        static std::list<Command*> deserialize(helpers::ByteQueue& data);
        
        virtual ~Command();
        
        void serialize(helpers::ByteQueue& data);
      protected:
        virtual void serialize_(helpers::ByteQueue& data) = 0;
      public:
        virtual char type() = 0;
    };
    
    class DuplicationCommand : public Command {
      public:
        uint32_t fileID;
        uint32_t srcPeer;
        uint32_t dstPeer;
        DuplicationCommand(helpers::ByteQueue& data);
        DuplicationCommand(uint32_t fileID, uint32_t srcPeer, uint32_t dstPeer);
      private:
        void serialize_(helpers::ByteQueue& data);
      public:
        char type();
    };
    
    class BalancingCommand : public Command {
      public:
        uint32_t fileID;
        uint32_t srcPeer;
        uint32_t peer1;
        uint32_t peer2;
        BalancingCommand(helpers::ByteQueue& data);
        BalancingCommand(uint32_t fileID, uint32_t srcPeer, uint32_t peer1, uint32_t peer2);
      private:
        void serialize_(helpers::ByteQueue& data);
      public:
        char type();
    };
  private:
    static Folder rootFolder;
    static uint32_t nextID;
    static uint32_t localIP;
    static std::set<uint32_t> storedFiles;
  public:
    static void init(uint32_t localIP);
    
    static helpers::ByteQueue serialize();
    static void deserialize(helpers::ByteQueue& data);
    
    static bool parseName(const std::string& name);
    static bool parsePath(const std::string& path);
    
    static Folder* createFolder(const std::string& fullPath);
    static Folder* retrieveFolder(const std::string& fullPath, std::string& foundPath);
    static Folder* updateFolder(const std::string& fullPath, const std::string& newName);
    static bool deleteFolder(const std::string& fullPath);
    
    static File* createFile(const std::string& fullPath, const std::string& author);
    static File* createFile(const std::string& fullPath, helpers::ByteQueue& info);
    static File* retrieveFile(const std::string& fullPath);
    static File* updateFile(const std::string& fullPath, const std::string& newName);
    static bool deleteFile(const std::string& fullPath);
    
    static uint32_t getTotalFolders();
    static uint32_t getTotalFiles();
    static uint64_t getTotalSize();
    
    static std::list<Command*> calculateDuplications(const std::set<uint32_t>& peers);
    static std::list<Command*> calculateBalance(const std::set<uint32_t>& peers);
    static void processCommands(const std::list<Command*>& cmds);
};

#endif /* FILESYSTEM_HPP_ */
