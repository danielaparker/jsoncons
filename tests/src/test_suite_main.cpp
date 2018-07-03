// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <stdlib.h>
#include <stdio.h>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(test_suite_main)
void *inc_x(void *p) {
  int *px = (int*)p;
  (*px)++;
  return NULL;
}

BOOST_AUTO_TEST_CASE(test_sanitize)
{
  char *p = (char*)malloc(10);
  p[5] = '\0';
  if (p[2] == '\0')  /* MSAN: uninitialized memory read */
    printf("found null\n");

  p[11] = '\0'; /* ASAN: heap-buffer-overflow */

  int i = 23;
  i <<= 32;  /* UBSAN: shift overflow */
  char array[3] = "ab";
  printf("one step beyond: %c\n", array[4]); /* UBSAN: index out of bounds*/
  char data[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
  int *pi = (int*)&(data[1]);
  printf("int %08x\n", *pi); /* UBSAN: misaligned address */

  p = NULL; /* LSAN: leak p */
}

BOOST_AUTO_TEST_SUITE_END()

