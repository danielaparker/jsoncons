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

BOOST_AUTO_TEST_CASE(test_longlong)
{
    long long n = 100;
    bignum val = n;

    std::cout << "long long " << val << std::endl;
    //std::cout << val.to_string(16) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_bignum2)
{
    std::string v = "10000000000000000";
    bignum val(v.data());

    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_CASE(test_logical_operations)
{
    bignum x( "888888888888888888" );
    bignum y( "888888888888888888" );

    bignum z = x & y;
    BOOST_CHECK( z == x );
}

BOOST_AUTO_TEST_CASE(test_addition)
{
    bignum x( "4444444444444444444444444444444" );
    bignum y( "4444444444444444444444444444444" );
    bignum a( "8888888888888888888888888888888" );

    bignum z = x + y;
    BOOST_CHECK( z == a );
}

BOOST_AUTO_TEST_CASE(test_multiplication)
{
    bignum x( "4444444444444444444444444444444" );
    bignum a( "8888888888888888888888888888888" );

    bignum z = 2*x;
    BOOST_CHECK( z == a );

    z = x*2;
    BOOST_CHECK( z == a );
}

BOOST_AUTO_TEST_SUITE_END()

