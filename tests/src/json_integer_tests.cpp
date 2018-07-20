// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_integer_tests)

BOOST_AUTO_TEST_CASE(test_integer_limits)
{
    {
        std::ostringstream os;

        os << "{\"max int64_t\":" << (std::numeric_limits<int64_t>::max)() << "}";
        json val = json::parse(os.str());
        BOOST_REQUIRE(val["max int64_t"].is_integer());
        BOOST_CHECK(val["max int64_t"].as<int64_t>() == (std::numeric_limits<int64_t>::max)());
    }
    {
        std::ostringstream os;

        os << "{\"min int64_t\":" << (std::numeric_limits<int64_t>::min)() << "}";
        json val = json::parse(os.str());
        BOOST_REQUIRE(val["min int64_t"].is_integer());
        BOOST_CHECK(val["min int64_t"].as<int64_t>() == (std::numeric_limits<int64_t>::min)());
    }

    // test overflow
    {
        std::ostringstream os;

        os << "{\"int overflow\":-" << (std::numeric_limits<int64_t>::max)() << "0}";
        json val = json::parse(os.str());
        BOOST_REQUIRE(val["int overflow"].is_bignum());
    }
    {
        std::ostringstream os;

        os << "{\"max uint64_t\":" << (std::numeric_limits<uint64_t>::max)() << "}";
        json val = json::parse(os.str());
        BOOST_REQUIRE(val["max uint64_t"].is_uinteger());
        BOOST_CHECK(val["max uint64_t"].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());
    }
    {
        std::ostringstream os;

        os << "{\"uint overflow\":" << (std::numeric_limits<uint64_t>::max)() << "0}";
        json val = json::parse(os.str());
        BOOST_REQUIRE(val["uint overflow"].is_bignum());
    }
}

BOOST_AUTO_TEST_SUITE_END()

