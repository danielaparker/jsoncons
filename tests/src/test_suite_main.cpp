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

BOOST_AUTO_TEST_SUITE_END()

