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

class System {
  private:
    static concurrency::Thread* thread;
    static bool started;
    static bool initialized;
    
    enum State {
      STATE_INIT,
      STATE_LOGIN,
      STATE_IDLE
    };
    
    State state;
    std::map<uint32_t, helpers::Timer> peers;
    std::map<uint32_t, std::string> users;
    network::Address localAddress;
    network::Address multicastAddress;
    network::UDPSocket mainUDPSocket;
    network::TCPServer mainTCPServer;
    network::TCPServer httpTCPServer;
  public:
    static bool start();
    static void stop(bool wait = false);
    static bool changing();
    static bool running();
  private:
    System();
    void run();
    
    void stateInit();
    void stateLogin();
    void stateIdle();
    
    void speak();
    void listen();
    void detectFailure();
    void httpServer();
};

#endif /* SYSTEM_HPP_ */
