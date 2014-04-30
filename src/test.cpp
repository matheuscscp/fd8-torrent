/*
 * test.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#include <cstdio>

#include "Thread.hpp"

static void foo() {
  printf("foo entrou\n");
  fflush(stdout);
  Thread::sleep(2500);
  printf("foo saiu\n");
}

int test() {
  Thread t(foo);
  t.start();
  t.join();
  t.join();
  printf("cabou\n");
  return 0;
}
