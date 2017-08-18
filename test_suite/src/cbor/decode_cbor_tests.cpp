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

BOOST_AUTO_TEST_SUITE(decode_cbor_tests)

void check_decode(const std::vector<uint8_t>& v, const json& expected)
{
    try
    {
        json result = decode_cbor<json>(v);
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
    check_decode({'\x00'},json(0U));
    check_decode({'\x01'},json(1U));
    check_decode({'\x0a'},json(10U));
    check_decode({'\x17'},json(23U));
    check_decode({'\x18','\x18'},json(24U));
    check_decode({'\x18',U'\xff'},json(255U));
    check_decode({'\x19','\x01','\x00'},json(256U));
    check_decode({'\x19',U'\xff',U'\xff'},json(65535U));
    check_decode({'\x1a',0,1,'\x00','\x00'},json(65536U));
    check_decode({'\x1a',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295U));
    check_decode({'\x1b',0,0,0,1,0,0,0,0},json(4294967296U));
    check_decode({'\x1b',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<uint64_t>::max()));

    // positive signed integer
    check_decode({'\x00'},json(0));
    check_decode({'\x01'},json(1));
    check_decode({'\x0a'},json(10));
    check_decode({'\x17'},json(23));
    check_decode({'\x18','\x18'},json(24));
    check_decode({'\x18',U'\xff'},json(255));
    check_decode({'\x19','\x01','\x00'},json(256));
    check_decode({'\x19',U'\xff',U'\xff'},json(65535));
    check_decode({'\x1a',0,1,'\x00','\x00'},json(65536));
    check_decode({'\x1a',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295));
    check_decode({'\x1b',0,0,0,1,0,0,0,0},json(4294967296));
    check_decode({'\x1b',U'\x7f',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<int64_t>::max()));

    // negative integers
    check_decode({'\x20'},json(-1));
    check_decode({'\x21'},json(-2));
    check_decode({'\x37'},json(-24));
    check_decode({'\x38','\x18'},json(-25));
    check_decode({'\x38',U'\xff'},json(-256));
    check_decode({'\x39','\x01','\x00'},json(-257));
    check_decode({'\x39',U'\xff',U'\xff'},json(-65536));
    check_decode({'\x3a',0,1,'\x00','\x00'},json(-65537));
    check_decode({'\x3a',U'\xff',U'\xff',U'\xff',U'\xff'},json(-4294967296));
    check_decode({'\x3b',0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_decode({U'\xf6'},json::null());
    check_decode({U'\xf5'},json(true));
    check_decode({U'\xf4'},json(false));

    // floating point
    check_decode({U'\xfb',0,0,0,0,0,0,0,0},json(0.0));
    check_decode({U'\xfb',U'\xbf',U'\xf0',0,0,0,0,0,0},json(-1.0));
    check_decode({U'\xfb',U'\xc1',U'\x6f',U'\xff',U'\xff',U'\xe0',0,0,0},json(-16777215.0));

    // string
    std::cout << "check 1" << std::endl;
    check_decode({U'\x60'},json(""));
    std::cout << "check 2" << std::endl;
    check_decode({U'\x61',' '},json(" "));
    std::cout << "check 3" << std::endl;
    check_decode({U'\x78','\x18','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));

    // strings with undefined length
    //check_decode({U'\x5f',U'\xff'}, json("h''"));
    //check_decode({U'\x7f',U'\xff'}, json(""));
    //check_decode({U'\x5f',U'\x40',U'\xff'}, json("h''"));
    //check_decode({U'\x7f',U'\x60',U'\xff'}, json(""));
    //check_decode({U'\x5f',U'\x40',U'\x40',U'\xff'}, json("h''"));
    check_decode({U'\x7f',U'\x60',U'\x60',U'\xff'}, json(""));
    //check_decode({U'\x5f',U'\x43Hel',U'\x42lo\xff'}, json("h'48656c6c6f'"));
    check_decode({U'\x7f',U'\x63','H','e','l',U'\x62','l','o',U'\xff'}, json("Hello"));
    //check_decode({U'\x5f',U'\x41H\',U'\x41""e\x41l\x41l\x41o\xff'}, json("h'48656c6c6f'"));
    check_decode({U'\x7f',U'\x61','H',U'\x61','e','\x61','l','\x61','l','\x61','o',U'\xff'}, json("Hello"));
    //check_decode({U'\x5f',U'\x41H\',U'\x41""e\x40\x41l\x41l\x41o\xff'}, json("h'48656c6c6f'"));
    check_decode({U'\x7f',U'\x61','H',U'\x61','e','\x61','l','\x60','\x61','l','\x61','o',U'\xff'}, json("Hello"));

}

BOOST_AUTO_TEST_CASE(cbor_arrays_and_maps)
{
    check_decode({U'\x80'},json::array());
    check_decode({U'\xa0'},json::object());

    check_decode({U'\x81','\0'},json::parse("[0]"));
    check_decode({U'\x82','\0','\0'},json::array({0,0}));
    check_decode({U'\x82',U'\x81','\0','\0'}, json::parse("[[0],0]"));
    check_decode({U'\x81','\x65','H','e','l','l','o'},json::parse("[\"Hello\"]"));

    check_decode({U'\xa1','\x62','o','c',U'\x81','\0'}, json::parse("{\"oc\": [0]}"));
    check_decode({U'\xa1','\x62','o','c',U'\x84','\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

BOOST_AUTO_TEST_SUITE_END()

