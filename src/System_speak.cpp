/*
 * System_speak.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <vector>

// local
#include "Defines.hpp"

using namespace std;
using namespace network;
using namespace helpers;

void System::speak() {
  static Timer timer;
  
  if (!timer.counting())
    timer.start();
  if (timer.time() > MS_SPEAK) {
    vector<char> data;
    string& name = users[localAddress.ip].name;
    uint32_t nameSize = name.size();
    data.insert(data.end(), (char*)&nameSize, ((char*)&nameSize) + 4);
    data.insert(data.end(), name.c_str(), name.c_str() + name.size());
    mainUDPSocket.send(multicastAddress, data);
    timer.start();
  }
}
