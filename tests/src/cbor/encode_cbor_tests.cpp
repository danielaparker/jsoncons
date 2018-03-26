// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

// test vectors from tinycbor https://github.com/01org/tinycbor tst_encoder.cpp
// MIT license

BOOST_AUTO_TEST_SUITE(cbor_tests)

void check_encode(const std::vector<uint8_t>& expected, const json& j)
{
    std::vector<uint8_t> result;
    encode_cbor(j,result);
    BOOST_REQUIRE_MESSAGE(expected.size() == result.size(), j.to_string());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        BOOST_REQUIRE_MESSAGE(expected[i] == result[i], j.to_string());
    }
}

BOOST_AUTO_TEST_CASE(cbor_encoder_test)
{
    // unsigned integer
    check_encode({0x00},json(0U));
    check_encode({0x01},json(1U));
    check_encode({0x0a},json(10U));
    check_encode({0x17},json(23U));
    check_encode({0x18,0x18},json(24U));
    check_encode({0x18,0xff},json(255U));
    check_encode({0x19,0x01,0x00},json(256U));
    check_encode({0x19,0xff,0xff},json(65535U));
    check_encode({0x1a,0,1,0x00,0x00},json(65536U));
    check_encode({0x1a,0xff,0xff,0xff,0xff},json(4294967295U));
    check_encode({0x1b,0,0,0,1,0,0,0,0},json(4294967296U));
    check_encode({0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<uint64_t>::max()));

    // positive signed integer
    check_encode({0x00},json(0));
    check_encode({0x01},json(1));
    check_encode({0x0a},json(10));
    check_encode({0x17},json(23));
    check_encode({0x18,0x18},json(24));
    check_encode({0x18,0xff},json(255));
    check_encode({0x19,0x01,0x00},json(256));
    check_encode({0x19,0xff,0xff},json(65535));
    check_encode({0x1a,0,1,0x00,0x00},json(65536));
    check_encode({0x1a,0xff,0xff,0xff,0xff},json(4294967295));
    check_encode({0x1b,0,0,0,1,0,0,0,0},json(4294967296));
    check_encode({0x1b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<int64_t>::max()));

    // negative integers
    check_encode({0x20},json(-1));
    check_encode({0x21},json(-2));
    check_encode({0x37},json(-24));
    check_encode({0x38,0x18},json(-25));
    check_encode({0x38,0xff},json(-256));
    check_encode({0x39,0x01,0x00},json(-257));
    check_encode({0x39,0xff,0xff},json(-65536));
    check_encode({0x3a,0,1,0x00,0x00},json(-65537));
    check_encode({0x3a,0xff,0xff,0xff,0xff},json(-4294967296));
    check_encode({0x3b,0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_encode({0xf6},json::null());
    check_encode({0xf5},json(true));
    check_encode({0xf4},json(false));

    // floating point
    check_encode({0xfb,0,0,0,0,0,0,0,0},json(0.0));
    check_encode({0xfb,0xbf,0xf0,0,0,0,0,0,0},json(-1.0));
    check_encode({0xfb,0xc1,0x6f,0xff,0xff,0xe0,0,0,0},json(-16777215.0));

    // byte string
    check_encode({0x40},json(byte_string()));
    check_encode({0x41,' '},json(byte_string({' '})));
    check_encode({0x41,0},json(byte_string({0})));
    check_encode({0x45,'H','e','l','l','o'},json(byte_string("Hello")));
    check_encode({0x58,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json(byte_string("123456789012345678901234")));

    // text string
    check_encode({0x60},json(""));
    check_encode({0x61,' '},json(" "));
    check_encode({0x78,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));


}

BOOST_AUTO_TEST_CASE(cbor_arrays_and_maps)
{
    check_encode({0x80},json::array());
    check_encode({0xa0},json::object());

    check_encode({0x81,'\0'},json::parse("[0]"));
    check_encode({0x82,'\0','\0'},json::array({0,0}));
    check_encode({0x82,0x81,'\0','\0'}, json::parse("[[0],0]"));
    check_encode({0x81,0x65,'H','e','l','l','o'},json::parse("[\"Hello\"]"));

    check_encode({0xa1,0x62,'o','c',0x81,'\0'}, json::parse("{\"oc\": [0]}"));
    check_encode({0xa1,0x62,'o','c',0x84,'\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

BOOST_AUTO_TEST_SUITE_END()

