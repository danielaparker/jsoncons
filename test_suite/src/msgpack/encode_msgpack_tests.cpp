// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::msgpack;

BOOST_AUTO_TEST_SUITE(msgpack_tests)

void check_encode(const std::vector<uint8_t>& expected, const json& j)
{
    std::vector<uint8_t> result = encode_msgpack(j);
    BOOST_REQUIRE_MESSAGE(expected.size() == result.size(), j.to_string());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        if (expected[i] != result[i])
        {
            for (size_t k = 0; k < expected.size(); ++k)
            {
                std::cout << std::hex << (int)expected[k] << " " << std::hex << (int)result[k] << std::endl;
            }
        }
        BOOST_REQUIRE_MESSAGE(expected[i] == result[i], j.to_string());
    }
}

BOOST_AUTO_TEST_CASE(msgpack_encoder_test)
{
    // positive fixint 0x00 - 0x7f
    check_encode({'\x00'},json(0U));
    check_encode({'\x01'},json(1U));
    check_encode({'\x0a'},json(10U));
    check_encode({'\x17'},json(23U));
    check_encode({'\x18'},json(24U));
    check_encode({'\x7f'},json(127U)); 

    check_encode({U'\xcc',U'\xff'},json(255U));
    check_encode({U'\xcd','\x01','\x00'},json(256U));
    check_encode({U'\xcd',U'\xff',U'\xff'},json(65535U));
    check_encode({U'\xce',0,1,'\x00','\x00'},json(65536U));
    check_encode({U'\xce',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295U));
    check_encode({U'\xcf',0,0,0,1,0,0,0,0},json(4294967296U));
    check_encode({U'\xcf',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<uint64_t>::max()));

    check_encode({'\x01'},json(1));
    check_encode({'\x0a'},json(10));
    check_encode({'\x17'},json(23)); 
    check_encode({'\x18'},json(24)); 
    check_encode({'\x7f'},json(127)); 

    check_encode({U'\xcc',U'\xff'},json(255));
    check_encode({U'\xcd','\x01','\x00'},json(256));
    check_encode({U'\xcd',U'\xff',U'\xff'},json(65535));
    check_encode({U'\xce',0,1,'\x00','\x00'},json(65536));
    check_encode({U'\xce',U'\xff',U'\xff',U'\xff',U'\xff'},json(4294967295));
    check_encode({U'\xd3',0,0,0,1,0,0,0,0},json(4294967296));
    check_encode({U'\xd3',U'\x7f',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff',U'\xff'},json(std::numeric_limits<int64_t>::max()));

    // negative fixint 0xe0 - 0xff
    check_encode({U'\xe0'},json(-32));
    check_encode({U'\xff'},json(-1)); //

    // negative integers
    check_encode({U'\xd1',U'\xff',U'\x00'},json(-256));
    //check_encode({U'\xd1','\x01','\x00'},json(-257));
    //check_encode({'\x39',U'\xff',U'\xff'},json(-65536));
    //check_encode({'\x3a',0,1,'\x00','\x00'},json(-65537));
    //check_encode({'\x3a',U'\xff',U'\xff',U'\xff',U'\xff'},json(-4294967296));
    //check_encode({'\x3b',0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_encode({U'\xc0'},json::null()); // 
    check_encode({U'\xc3'},json(true)); //
    check_encode({U'\xc2'},json(false)); //

    // floating point
    check_encode({U'\xcb',0,0,0,0,0,0,0,0},json(0.0));
    check_encode({U'\xcb',U'\xbf',U'\xf0',0,0,0,0,0,0},json(-1.0));
    check_encode({U'\xcb',U'\xc1',U'\x6f',U'\xff',U'\xff',U'\xe0',0,0,0},json(-16777215.0));

    // string
    check_encode({U'\xd9'},json(""));
    check_encode({U'\xd9',' '},json(" "));
    check_encode({U'\xd9','\x18','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));


}

BOOST_AUTO_TEST_CASE(msgpack_arrays_and_maps)
{
    // fixarray
    check_encode({U'\x90'},json::array());
    check_encode({U'\x80'},json::object());

    check_encode({U'\x91','\0'},json::parse("[0]"));
    check_encode({U'\x92','\0','\0'},json::array({0,0}));
    check_encode({U'\x92',U'\x91','\0','\0'}, json::parse("[[0],0]"));
    check_encode({U'\x91',0xa5,'H','e','l','l','o'},json::parse("[\"Hello\"]"));

    check_encode({U'\x81',0xa2,'o','c',U'\x91','\0'}, json::parse("{\"oc\": [0]}"));
    check_encode({U'\x81',0xa2,'o','c',U'\x94','\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

BOOST_AUTO_TEST_SUITE_END()

