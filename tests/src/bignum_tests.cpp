// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <jsoncons/bignum.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(bignum_tests)

BOOST_AUTO_TEST_CASE(test_bignum)
{
    bignum val{};
}

BOOST_AUTO_TEST_SUITE_END()

