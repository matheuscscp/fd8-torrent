/*
 * test.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Pimenta
 */

#include "Thread.hpp"

static void foo() {
  printf("foo entrou\n");
  fflush(stdout);
  Thread_sleep(2500, nullptr);
  printf("foo saiu\n");
}

int test() {
  Thread<foo> t;
  t.start();
  t.join();
  t.join();
  printf("cabou\n");
  return 0;
}
