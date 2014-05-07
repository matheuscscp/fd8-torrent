/*
 * Platform.cpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

// this
#include "Platform.hpp"

// standard
#include <cstdlib>
#include <cstdio>

// local
#include "Define.hpp"

void Platform::browser() {
  char cmd[100];
#ifdef _WIN32
  sprintf(cmd, "start http://localhost:%d", TCP_WEB_HOST);
#else
  sprintf(cmd, "sensible-browser http://localhost:%d", TCP_WEB_HOST);
#endif
  system(cmd);
}
