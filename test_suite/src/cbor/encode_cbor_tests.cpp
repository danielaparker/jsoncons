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

BOOST_AUTO_TEST_SUITE(cbor_tests)

void check_encode(const std::vector<uint8_t>& expected, const json& j)
{
    std::vector<uint8_t> result = encode_cbor(j);
    BOOST_REQUIRE_MESSAGE(expected.size() == result.size(), j.to_string());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        BOOST_REQUIRE_MESSAGE(expected[i] == result[i], j.to_string());
    }
}

BOOST_AUTO_TEST_CASE(cbor_encoder_test)
{
    // unsigned integer
    check_encode({'\x00'},json(0U));
    check_encode({'\x01'},json(1U));
    check_encode({'\x0a'},json(10U));
    check_encode({'\x17'},json(23U));
    check_encode({'\x18','\x18'},json(24U));
    check_encode({'\x18',U'\xff'},json(255U));
    check_encode({'\x19','\x01','\x00'},json(256U));
    check_encode({'\x19',U'\xff',U'\xff'},json(65535U));
    check_encode({'\x1a',0,1,'\x00','\x00'},json(65536U));
    check_encode({'\x1a',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295U));
    check_encode({'\x1b',0,0,0,1,0,0,0,0},json(4294967296U));
    check_encode({'\x1b',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<uint64_t>::max()));

    // positive signed integer
    check_encode({'\x00'},json(0));
    check_encode({'\x01'},json(1));
    check_encode({'\x0a'},json(10));
    check_encode({'\x17'},json(23));
    check_encode({'\x18','\x18'},json(24));
    check_encode({'\x18',U'\xff'},json(255));
    check_encode({'\x19','\x01','\x00'},json(256));
    check_encode({'\x19',U'\xff',U'\xff'},json(65535));
    check_encode({'\x1a',0,1,'\x00','\x00'},json(65536));
    check_encode({'\x1a',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295));
    check_encode({'\x1b',0,0,0,1,0,0,0,0},json(4294967296));
    check_encode({'\x1b',U'\x7f',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<int64_t>::max()));

    // negative integers
    check_encode({'\x20'},json(-1));
    check_encode({'\x21'},json(-2));
    check_encode({'\x37'},json(-24));
    check_encode({'\x38','\x18'},json(-25));
    check_encode({'\x38',U'\xff'},json(-256));
    check_encode({'\x39','\x01','\x00'},json(-257));
    check_encode({'\x39',U'\xff',U'\xff'},json(-65536));
    check_encode({'\x3a',0,1,'\x00','\x00'},json(-65537));
    check_encode({'\x3a',U'\xff',U'\xff',U'\xff',U'\xff'},json(-4294967296));
    check_encode({'\x3b',0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_encode({U'\xf6'},json::null());
    check_encode({U'\xf5'},json(true));
    check_encode({U'\xf4'},json(false));

    // floating point
    check_encode({U'\xfb',0,0,0,0,0,0,0,0},json(0.0));
    check_encode({U'\xfb',U'\xbf',U'\xf0',0,0,0,0,0,0},json(-1.0));
    check_encode({U'\xfb',U'\xc1',U'\x6f',U'\xff',U'\xff',U'\xe0',0,0,0},json(-16777215.0));

    // string
    check_encode({U'\x60'},json(""));
    check_encode({U'\x61',' '},json(" "));
    check_encode({U'\x78','\x18','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));


}

BOOST_AUTO_TEST_CASE(cbor_arrays_and_maps)
{
    check_encode({U'\x80'},json::array());
    check_encode({U'\xa0'},json::object());

    check_encode({U'\x81','\0'},json::parse("[0]"));
    check_encode({U'\x82','\0','\0'},json::array({0,0}));
    check_encode({U'\x82',U'\x81','\0','\0'}, json::parse("[[0],0]"));
    check_encode({U'\x81','\x65','H','e','l','l','o'},json::parse("[\"Hello\"]"));

    check_encode({U'\xa1','\x62','o','c',U'\x81','\0'}, json::parse("{\"oc\": [0]}"));
    check_encode({U'\xa1','\x62','o','c',U'\x84','\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

BOOST_AUTO_TEST_SUITE_END()

