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
#include <set>
#include <string>

// local
#include "Helpers.hpp"
#include "Network.hpp"

class System {
  private:
    static bool started;
    static bool initialized;
    
    std::map<uint32_t, helpers::Timer> peers;
    std::set<std::string> users;
    network::Address localAddress;
    network::Address multicastAddress;
    network::UDPSocket mainUDPSocket;
  public:
    static bool start();
    static bool stop();
    static bool changing();
    static bool running();
  private:
    System();
    void run();
    void speak();
    void listen();
    void detectFailure();
    void httpServer();
    void httpServer_dataRequest(char* cRequest, char* buffer);
};

#endif /* SYSTEM_HPP_ */
