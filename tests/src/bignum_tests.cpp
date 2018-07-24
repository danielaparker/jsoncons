// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(bignum_tests)

BOOST_AUTO_TEST_CASE(test_positive_bignum)
{
    std::string expected = "18446744073709551616";
    std::vector<uint8_t> v = {1,0,0,0,0,0,0,0,0};
    bignum x(1,v.data(),v.size());

    std::string sx;
    x.dump(sx);
    BOOST_CHECK_EQUAL(expected,sx);

    bignum y(x);
    std::string sy;
    y.dump(sy);
    BOOST_CHECK_EQUAL(expected,sy);

    bignum z;
    z = x;
    std::string sz;
    y.dump(sz);
    BOOST_CHECK_EQUAL(expected,sz);
}

BOOST_AUTO_TEST_CASE(test_negative_bignum)
{
    std::string expected = "-18446744073709551617";
    std::vector<uint8_t> b = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum x(-1,b.data(),b.size());

    std::string sx;
    x.dump(sx);
    BOOST_CHECK_EQUAL(expected,sx);

    bignum y(x);
    std::string sy;
    y.dump(sy);
    BOOST_CHECK_EQUAL(expected,sy);

    bignum z;
    z = x;
    std::string sz;
    y.dump(sz);
    BOOST_CHECK_EQUAL(expected,sz);

    int signum;
    std::vector<uint8_t> v;
    x.dump(signum, v);

    BOOST_REQUIRE(v.size() == b.size());
    for (size_t i = 0; i < v.size(); ++i)
    {
        BOOST_REQUIRE(v[i] == b[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_longlong)
{
    long long n = std::numeric_limits<long long>::max();

    bignum val = n;

    //std::cout << "long long " << n << " == " << val << std::endl;
    //std::cout << val.to_string(16) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_bignum2)
{
    std::string v = "10000000000000000";
    bignum val(v.data());

    //std::cout << val << std::endl;
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

BOOST_AUTO_TEST_CASE(test_conversion_0)
{
    bignum x(1, {});

    json j(x);

    bignum y = j.as<bignum>();
    BOOST_CHECK_EQUAL(x,y);

    std::string s;
    y.dump(s);

    BOOST_CHECK(s == "0");
}

BOOST_AUTO_TEST_CASE(test_traits1)
{
    bignum x(1, {0x01,0x00});

    json j(x);

    bignum y = j.as<bignum>();
    BOOST_CHECK(x == y);

    std::string s;
    y.dump(s);

    BOOST_CHECK(s == "256");
}

BOOST_AUTO_TEST_CASE(test_traits2)
{
    bignum x(1, {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00});

    json j(x);

    bignum y = j.as<bignum>();
    BOOST_CHECK(x == y);

    std::string s;
    y.dump(s);

    BOOST_CHECK(s == "18446744073709551616");
}

BOOST_AUTO_TEST_CASE(test_traits3)
{
    bignum x(-1, {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00});

    int signum;
    std::vector<uint8_t> v;
    x.dump(signum,v);

    BOOST_REQUIRE(signum == -1);
    for (auto c : v)
    {
        //std::cout << std::hex << (int)c;
    }
    //std::cout << std::endl;

    json j(x);

    bignum y = j.as<bignum>();
    BOOST_CHECK(x == y);

    std::string s;
    y.dump(s);

    BOOST_CHECK(s == "-18446744073709551617");
}
BOOST_AUTO_TEST_SUITE_END()

