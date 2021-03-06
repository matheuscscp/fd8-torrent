/*
 * System.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

// local
#include "Concurrency.hpp"
#include "Network.hpp"
#include "FileSystem.hpp"

class System {
  private:
    static concurrency::Thread* thread;
    static bool started;
    static bool initialized;
    
    class User {
      public:
        uint32_t sessionID;
        std::string name;
        helpers::Timer timer;
        User();
        User(const std::string& name);
    };
    
    enum State {
      STATE_NONE,
      STATE_LOGIN,
      STATE_IDLE
    };
    
    uint32_t nextSessionID;
    State state, newState;
    std::map<uint32_t, User> users;
    network::Address localAddress;
    network::Address broadcastAddress;
    network::Address multicastAddress;
    network::UDPSocket mainUDPSocket;
    network::TCPServer mainTCPServer;
    network::TCPServer httpTCPServer;
    helpers::Timer loginSyncTimer;
    helpers::Timer idleBalancingTimer;
    concurrency::Thread httpThread;
    uint32_t downloadsRemaining;
  public:
    static bool start();
    static void stop(bool wait = false);
    static bool changing();
    static bool running();
  private:
    System();
    ~System();
    
    void run();
    void change();
    
    void changeToLogin();
    void stateLogin();
    
    void changeToIdle();
    void stateIdle();
    
    void executeProtocol();
    
    void speak();
    void listen();
    
    void detectFailure();
    void recoverFromFailure();
    
    void loginHttpServer();
    void loginHttpServer_loginAttempt(network::TCPConnection* client, const std::string& request);
    
    void httpServer();
    void httpServer_dataRequest(network::TCPConnection* client, const std::string& request);
    void httpServer_recvFile(network::TCPConnection* client);
    
    // initial synchronization
    void requestSystemState();
    void respondSystemState(network::TCPConnection* peer);
    
    // folder synchronization
    void send_createFolder(const std::string& fullPath);
    void send_updateFolder(const std::string& fullPath, const std::string& newName);
    void send_deleteFolder(const std::string& fullPath);
    void recv_createFolder(const std::string& fullPath);
    void recv_updateFolder(const std::string& fullPath, const std::string& newName);
    void recv_deleteFolder(const std::string& fullPath);
    
    // file synchronization
    void send_createFile(const std::string& fullPath, const helpers::ByteQueue& info);
    void send_updateFile(const std::string& fullPath, const std::string& newName);
    void send_deleteFile(const std::string& fullPath);
    void recv_createFile(const std::string& fullPath, helpers::ByteQueue& info);
    void recv_updateFile(const std::string& fullPath, const std::string& newName);
    void recv_deleteFile(const std::string& fullPath);
    void send_files(const std::list<FileSystem::Command*>& cmds);
};

#endif /* SYSTEM_HPP_ */
