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

// test vectors from tinycbor https://github.com/01org/tinycbor tst_decoder.cpp
// MIT license

BOOST_AUTO_TEST_SUITE(decode_cbor_tests)

void check_decode(const std::vector<uint8_t>& v, const json& expected)
{
    try
    {
        json result = decode_cbor<json>(v);
        BOOST_REQUIRE_MESSAGE(expected == result, expected.to_string());
        BOOST_REQUIRE_MESSAGE(expected == result, expected.to_string());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << expected.to_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(cbor_decode_test)
{
    // unsigned integer
    check_decode({0x00},json(0U));
    check_decode({0x01},json(1U));
    check_decode({0x0a},json(10U));
    check_decode({0x17},json(23U));
    check_decode({0x18,0x18},json(24U));
    check_decode({0x18,0xff},json(255U));
    check_decode({0x19,0x01,0x00},json(256U));
    check_decode({0x19,0xff,0xff},json(65535U));
    check_decode({0x1a,0,1,0x00,0x00},json(65536U));
    check_decode({0x1a,0xff,0xff,0xff,0xff},json(4294967295U));
    check_decode({0x1b,0,0,0,1,0,0,0,0},json(4294967296U));
    check_decode({0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<uint64_t>::max()));

    // positive signed integer
    check_decode({0x00},json(0));
    check_decode({0x01},json(1));
    check_decode({0x0a},json(10));
    check_decode({0x17},json(23));
    check_decode({0x18,0x18},json(24));
    check_decode({0x18,0xff},json(255));
    check_decode({0x19,0x01,0x00},json(256));
    check_decode({0x19,0xff,0xff},json(65535));
    check_decode({0x1a,0,1,0x00,0x00},json(65536));
    check_decode({0x1a,0xff,0xff,0xff,0xff},json(4294967295));
    check_decode({0x1b,0,0,0,1,0,0,0,0},json(4294967296));
    check_decode({0x1b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<int64_t>::max()));

    // negative integers
    check_decode({0x20},json(-1));
    check_decode({0x21},json(-2));
    check_decode({0x37},json(-24));
    check_decode({0x38,0x18},json(-25));
    check_decode({0x38,0xff},json(-256));
    check_decode({0x39,0x01,0x00},json(-257));
    check_decode({0x39,0xff,0xff},json(-65536));
    check_decode({0x3a,0,1,0x00,0x00},json(-65537));
    check_decode({0x3a,0xff,0xff,0xff,0xff},json(-4294967296));
    check_decode({0x3b,0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_decode({0xf6},json::null());
    check_decode({0xf5},json(true));
    check_decode({0xf4},json(false));

    // floating point
    check_decode({0xfb,0,0,0,0,0,0,0,0},json(0.0));
    check_decode({0xfb,0xbf,0xf0,0,0,0,0,0,0},json(-1.0));
    check_decode({0xfb,0xc1,0x6f,0xff,0xff,0xe0,0,0,0},json(-16777215.0));

    // byte string
    std::vector<uint8_t> v;
    check_decode({0x40},json(v.data(),v.size()));
    v = {' '};
    check_decode({0x41,' '},json(v.data(),v.size()));
    v = {0};
    check_decode({0x41,0},json(v.data(),v.size()));
    v = {'H','e','l','l','o'};
    check_decode({0x45,'H','e','l','l','o'},json(v.data(),v.size()));
    v = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'};
    check_decode({0x58,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json(v.data(),v.size()));

    // string
    check_decode({0x60},json(""));
    check_decode({0x61,' '},json(" "));
    check_decode({0x78,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));

    // byte strings with undefined length
    std::vector<uint8_t> bs = {};
    check_decode({0x5f,0xff}, json(bs.data(),bs.size()));
    check_decode({0x5f,0x40,0xff}, json(bs.data(),bs.size()));
    check_decode({0x5f,0x40,0x40,0xff}, json(bs.data(),bs.size()));

    bs = {'H','e','l','l','o'};
    check_decode({0x5f,0x43,'H','e','l',0x42,'l','o',0xff}, json(bs.data(),bs.size()));
    check_decode({0x5f,0x41,'H',0x41,'e',0x41,'l',0x41,'l',0x41,'o',0xff}, json(bs.data(),bs.size()));
    check_decode({0x5f,0x41,'H',0x41,'e',0x40,0x41,'l',0x41,'l',0x41,'o',0xff}, json(bs.data(),bs.size()));

    // text strings with undefined length
    check_decode({0x7f,0xff}, json(""));
    check_decode({0x7f,0x60,0xff}, json(""));
    check_decode({0x7f,0x60,0x60,0xff}, json(""));

    check_decode({0x7f,0x63,'H','e','l',0x62,'l','o',0xff}, json("Hello"));
    check_decode({0x7f,0x61,'H',0x61,'e',0x61,'l',0x61,'l',0x61,'o',0xff}, json("Hello"));
    check_decode({0x7f,0x61,'H',0x61,'e',0x61,'l',0x60,0x61,'l',0x61,'o',0xff}, json("Hello"));

}

BOOST_AUTO_TEST_CASE(cbor_arrays_and_maps)
{
    check_decode({0x80},json::array());
    check_decode({0xa0},json::object());

    check_decode({0x81,'\0'},json::parse("[0]"));
    check_decode({0x82,'\0','\0'},json::array({0,0}));
    check_decode({0x82,0x81,'\0','\0'}, json::parse("[[0],0]"));
    check_decode({0x81,0x65,'H','e','l','l','o'},json::parse("[\"Hello\"]"));

    check_decode({0xa1,0x62,'o','c',0x81,'\0'}, json::parse("{\"oc\": [0]}"));
    check_decode({0xa1,0x62,'o','c',0x84,'\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

BOOST_AUTO_TEST_SUITE_END()

