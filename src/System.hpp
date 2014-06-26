/*
 * System.hpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

// standard
#include <map>
#include <string>

// local
#include "Concurrency.hpp"
#include "Helpers.hpp"
#include "Network.hpp"
#include "User.hpp"

class System {
  private:
    static concurrency::Thread* thread;
    static bool started;
    static bool initialized;
    
    enum State {
      STATE_NONE,
      STATE_LOGIN,
      STATE_IDLE
    };
    
    State state;
    std::map<uint32_t, User> users;
    network::Address localAddress;
    network::Address multicastAddress;
    network::UDPSocket mainUDPSocket;
    network::TCPServer mainTCPServer;
    network::TCPServer httpTCPServer;
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
    
    void changeToLogin();
    void stateLogin();
    
    void changeToIdle();
    void stateIdle();
    
    void speak();
    void executeProtocol();
    void listen();
    void detectFailure();
    
    void loginHttpServer();
    void loginHttpServer_loginAttempt(const std::string& data);
    
    void httpServer();
    void httpServer_dataRequest(const std::string& cRequest);
    
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
};

#endif /* SYSTEM_HPP_ */
