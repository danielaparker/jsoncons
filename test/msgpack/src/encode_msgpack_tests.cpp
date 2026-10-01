// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <jsoncons/json.hpp>
#include <vector>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

void check_encode_msgpack(const std::vector<uint8_t>& expected, 
                          const jsoncons::json& j)
{
    std::vector<uint8_t> result;
    msgpack::encode_msgpack(j, result);
    if (result.size() != expected.size())
    {
        std::cout << std::hex << (int)expected[0] << " " << std::hex << (int)result[0] << '\n';
    }
    REQUIRE(result.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        if (expected[i] != result[i])
        {
            std::cout << "Different " << i << "\n"; 
            for (std::size_t k = 0; k < expected.size(); ++k)
            {
                std::cout << std::hex << (int)expected[k] << " " << std::hex << (int)result[k] << '\n';
            }
        }
        REQUIRE(result[i] == expected[i]);
    }
}

TEST_CASE("encode_msgpack_test")
{
    // positive fixint 0x00 - 0x7f
    check_encode_msgpack({0x00},jsoncons::json(0U));
    check_encode_msgpack({0x01},jsoncons::json(1U));
    check_encode_msgpack({0x0a},jsoncons::json(10U));
    check_encode_msgpack({0x17},jsoncons::json(23U));
    check_encode_msgpack({0x18},jsoncons::json(24U));
    check_encode_msgpack({0x7f},jsoncons::json(127U)); 

    check_encode_msgpack({0xcc,0xff},jsoncons::json(255U));
    check_encode_msgpack({0xcd,0x01,0x00},jsoncons::json(256U));
    check_encode_msgpack({0xcd,0xff,0xff},jsoncons::json(65535U));
    check_encode_msgpack({0xce,0,1,0x00,0x00},jsoncons::json(65536U));
    check_encode_msgpack({0xce,0xff,0xff,0xff,0xff},jsoncons::json(4294967295U));
    check_encode_msgpack({0xcf,0,0,0,1,0,0,0,0},jsoncons::json(4294967296U));
    check_encode_msgpack({0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<uint64_t>::max)()));

    check_encode_msgpack({0x01},jsoncons::json(1));
    check_encode_msgpack({0x0a},jsoncons::json(10));
    check_encode_msgpack({0x17},jsoncons::json(23)); 
    check_encode_msgpack({0x18},jsoncons::json(24)); 
    check_encode_msgpack({0x7f},jsoncons::json(127)); 

    check_encode_msgpack({0xcc,0xff},jsoncons::json(255));
    check_encode_msgpack({0xcd,0x01,0x00},jsoncons::json(256));
    check_encode_msgpack({0xcd,0xff,0xff},jsoncons::json(65535));
    check_encode_msgpack({0xce,0,1,0x00,0x00},jsoncons::json(65536));
    check_encode_msgpack({0xce,0xff,0xff,0xff,0xff},jsoncons::json(4294967295));
    check_encode_msgpack({0xcf,0,0,0,1,0,0,0,0},jsoncons::json(4294967296));
    check_encode_msgpack({0xcf,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<int64_t>::max)()));

    // negative fixint 0xe0 - 0xff
    check_encode_msgpack({0xe0},jsoncons::json(-32));
    check_encode_msgpack({0xff},jsoncons::json(-1)); //

    // negative integers
    check_encode_msgpack({0xd1,0xff,0},jsoncons::json(-256));
    check_encode_msgpack({0xd1,0xfe,0xff},jsoncons::json(-257));
    check_encode_msgpack({0xd2,0xff,0xff,0,0},jsoncons::json(-65536));
    check_encode_msgpack({0xd2,0xff,0xfe,0xff,0xff},jsoncons::json(-65537));
    check_encode_msgpack({0xd3,0xff,0xff,0xff,0xff,0,0,0,0},jsoncons::json(-4294967296));
    check_encode_msgpack({0xd3,0xff,0xff,0xff,0xfe,0xff,0xff,0xff,0xff},jsoncons::json(-4294967297));

    // null, true, false
    check_encode_msgpack({0xc0},jsoncons::json::null()); // 
    check_encode_msgpack({0xc3},jsoncons::json(true)); //
    check_encode_msgpack({0xc2},jsoncons::json(false)); //

    // floating point
    check_encode_msgpack({0xca,0,0,0,0},jsoncons::json(0.0));
    check_encode_msgpack({0xca,0xbf,0x80,0,0},jsoncons::json(-1.0));
    check_encode_msgpack({0xca,0xcb,0x7f,0xff,0xff},jsoncons::json(-16777215.0));

    // string
    check_encode_msgpack({0xa0},jsoncons::json(""));
    check_encode_msgpack({0xa1,' '},jsoncons::json(" "));
    check_encode_msgpack({0xbf,'1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1'},
                 jsoncons::json("1234567890123456789012345678901"));
    check_encode_msgpack({0xd9,0x20,'1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2'},
                 jsoncons::json("12345678901234567890123456789012"));


}

TEST_CASE("encode_msgpack_arrays_and_maps")
{
    // fixarray
    check_encode_msgpack({0x90}, jsoncons::json(jsoncons::json_array_arg));
    check_encode_msgpack({0x80},jsoncons::json());

    check_encode_msgpack({0x91,'\0'},jsoncons::json::parse("[0]"));
    check_encode_msgpack({0x92,'\0','\0'}, jsoncons::json(jsoncons::json_array_arg, {0,0}));
    check_encode_msgpack({0x92,0x91,'\0','\0'}, jsoncons::json::parse("[[0],0]"));
    check_encode_msgpack({0x91,0xa5,'H','e','l','l','o'},jsoncons::json::parse("[\"Hello\"]"));

    check_encode_msgpack({0x81,0xa2,'o','c',0x91,'\0'}, jsoncons::json::parse("{\"oc\": [0]}"));
    check_encode_msgpack({0x81,0xa2,'o','c',0x94,'\0','\1','\2','\3'}, jsoncons::json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

namespace { namespace ns {

    struct Person
    {
        std::string name;
    };

}}

JSONCONS_ALL_MEMBER_TRAITS(ns::Person, name)

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <common/mock_stateful_allocator.hpp>
#include <scoped_allocator>

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

TEST_CASE("encode_msgpack allocator_set overloads")
{
    MyScopedAllocator<char> temp_alloc(1);

    auto aset = make_alloc_set(jsoncons::temp_alloc_arg, temp_alloc);

    SECTION("json, stream")
    {
        jsoncons::json person;
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        msgpack::encode_msgpack(aset, person, ss);
        jsoncons::json other = msgpack::decode_msgpack<jsoncons::json>(aset, ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        msgpack::encode_msgpack(aset, person, ss);
        ns::Person other = msgpack::decode_msgpack<ns::Person>(aset, ss);
        CHECK(other.name == person.name);
    }
}

#endif
