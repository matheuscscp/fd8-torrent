/*
 * main.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: Pimenta
 */

#include <SDL_net.h>
#include <cstdio>

int main(int argc, char* argv[]) {
  SDL_version compile_version;
  const SDL_version *link_version=SDLNet_Linked_Version();
  SDL_NET_VERSION(&compile_version);
  printf("compiled with SDL_net version: %d.%d.%d\n", 
          compile_version.major,
          compile_version.minor,
          compile_version.patch);
  printf("running with SDL_net version: %d.%d.%d\n", 
          link_version->major,
          link_version->minor,
          link_version->patch);
  
  return 0;
}
