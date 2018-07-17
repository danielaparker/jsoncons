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
    // 18446744073709551616
    std::vector<uint8_t> v = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    bignum val(1,v.data(),v.size());

    std::cout << val << std::endl;
    //std::cout << val.to_string(16) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_bignum2)
{
    std::string v = "10000000000000000";
    bignum val(v.data());

    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

