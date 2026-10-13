// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

void check_encode_ubjson(const std::vector<uint8_t>& expected, const jsoncons::json& j)
{
    std::vector<uint8_t> result;
    ubjson::encode_ubjson(j, result);
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

void check_encode_ubjson(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& result)
{
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

TEST_CASE("encode_ubjson_test")
{
    check_encode_ubjson({'U',0x00},jsoncons::json(0U));
    check_encode_ubjson({'U',0x01},jsoncons::json(1U));
    check_encode_ubjson({'U',0x0a},jsoncons::json(10U));
    check_encode_ubjson({'U',0x17},jsoncons::json(23U));
    check_encode_ubjson({'U',0x18},jsoncons::json(24U));
    check_encode_ubjson({'U',0x7f},jsoncons::json(127U)); 
    check_encode_ubjson({'U',0xff},jsoncons::json(255U));
    check_encode_ubjson({'I',0x01,0x00},jsoncons::json(256U));
    check_encode_ubjson({'l',0,1,0x00,0x00},jsoncons::json(65536U));
    check_encode_ubjson({'L',0,0,0,1,0,0,0,0},jsoncons::json(4294967296U));

    check_encode_ubjson({'U',0x01},jsoncons::json(1));
    check_encode_ubjson({'U',0x0a},jsoncons::json(10));
    check_encode_ubjson({'U',0x17},jsoncons::json(23)); 
    check_encode_ubjson({'U',0x18},jsoncons::json(24)); 
    check_encode_ubjson({'U',0x7f},jsoncons::json(127)); 

    check_encode_ubjson({'U',0xff},jsoncons::json(255));
    check_encode_ubjson({'I',0x01,0x00},jsoncons::json(256));
    check_encode_ubjson({'l',0,1,0x00,0x00},jsoncons::json(65536));
    check_encode_ubjson({'L',0,0,0,1,0,0,0,0},jsoncons::json(4294967296));
    check_encode_ubjson({'L',0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<int64_t>::max)()));

    check_encode_ubjson({'i',0xe0},jsoncons::json(-32));
    check_encode_ubjson({'i',0xff},jsoncons::json(-1)); //

    // negative integers
    check_encode_ubjson({'I',0xff,0},jsoncons::json(-256));
    check_encode_ubjson({'I',0xfe,0xff},jsoncons::json(-257));
    check_encode_ubjson({'l',0xff,0xff,0,0},jsoncons::json(-65536));
    check_encode_ubjson({'l',0xff,0xfe,0xff,0xff},jsoncons::json(-65537));
    check_encode_ubjson({'L',0xff,0xff,0xff,0xff,0,0,0,0},jsoncons::json(-4294967296));
    check_encode_ubjson({'L',0xff,0xff,0xff,0xfe,0xff,0xff,0xff,0xff},jsoncons::json(-4294967297));

    // null, true, false
    check_encode_ubjson({'Z'},jsoncons::json::null()); // 
    check_encode_ubjson({'T'},jsoncons::json(true)); //
    check_encode_ubjson({'F'},jsoncons::json(false)); //

    // floating point
    check_encode_ubjson({'d',0,0,0,0},jsoncons::json(0.0));
    check_encode_ubjson({'d',0xbf,0x80,0,0},jsoncons::json(-1.0));
    check_encode_ubjson({'d',0xcb,0x7f,0xff,0xff},jsoncons::json(-16777215.0));

    // string
    check_encode_ubjson({'S','U',0x00},jsoncons::json(""));
    check_encode_ubjson({'S','U',0x01,' '},jsoncons::json(" "));
    check_encode_ubjson({'S','U',0x1f,'1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1'},
                 jsoncons::json("1234567890123456789012345678901"));
    check_encode_ubjson({'S','U',0x20,'1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2'},
                 jsoncons::json("12345678901234567890123456789012"));
}
TEST_CASE("encode_ubjson uint64 above int64 max")
{
    check_encode_ubjson({'L',0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
                        jsoncons::json(uint64_t((std::numeric_limits<int64_t>::max)())));

    // UBJSON has no uint64 type, so larger values are written as high-precision numbers
    std::vector<uint8_t> expected = {'[','#','U',0x02,
                                     'H','U',19,'9','2','2','3','3','7','2','0','3','6','8','5','4','7','7','5','8','0','8',
                                     'H','U',20,'1','8','4','4','6','7','4','4','0','7','3','7','0','9','5','5','1','6','1','5'};
    std::vector<uint8_t> buffer;
    ubjson::encode_ubjson(jsoncons::json::parse("[9223372036854775808,18446744073709551615]"), buffer);
    CHECK(buffer == expected);

    auto j = ubjson::decode_ubjson<jsoncons::json>(buffer);
    REQUIRE(j.size() == 2);
    CHECK(j[0].as<uint64_t>() == 9223372036854775808u);
    CHECK(j[1].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());
}

TEST_CASE("encode_ubjson_arrays_and_maps")
{
    check_encode_ubjson({'[','#','U',0x00}, jsoncons::json(jsoncons::json_array_arg));
    check_encode_ubjson({'{','#','U',0x00},jsoncons::json());
    check_encode_ubjson({'[','#','U',0x01,'U',0x00},jsoncons::json::parse("[0]"));
    check_encode_ubjson({'[','#','U',0x02,'U',0x00,'U',0x00},jsoncons::json::parse("[0,0]"));
    check_encode_ubjson({'[','#','U',0x02,
                         '[','#','U',0x01,'U',0x00,
                         'U',0x00},jsoncons::json::parse("[[0],0]"));
    check_encode_ubjson({'[','#','U',0x01,'S','U',0x05,'H','e','l','l','o'},jsoncons::json::parse("[\"Hello\"]"));
    check_encode_ubjson({'{','#','U',0x01,'U',0x02,'o','c','[','#','U',0x01,'U',0x00}, jsoncons::json::parse("{\"oc\": [0]}"));
    check_encode_ubjson({'{','#','U',0x01,'U',0x02,'o','c','[','#','U',0x04,'U',0x00,'U',0x01,'U',0x02,'U',0x03}, jsoncons::json::parse("{\"oc\": [0,1,2,3]}"));
}

TEST_CASE("encode indefinite length ubjson arrays and maps")
{
    std::vector<uint8_t> v;
    ubjson::ubjson_bytes_encoder encoder(v);

    SECTION("[\"Hello\"]")
    {
        encoder.begin_array();
        encoder.string_value("Hello");
        encoder.end_array();

        check_encode_ubjson({'[','S','U',0x05,'H','e','l','l','o',']'}, v);
    }

    SECTION("{\"oc\": [0]}")
    {
        encoder.begin_object();
        encoder.key("oc");
        encoder.begin_array();
        encoder.uint64_value(0);
        encoder.end_array();
        encoder.end_object();

        check_encode_ubjson({'{','U',0x02,'o','c','[','U',0x00,']','}'}, v);
    }

    SECTION("{\"oc\": [0,1,2,3]}")
    {
        encoder.begin_object();
        encoder.key("oc");
        encoder.begin_array();
        encoder.uint64_value(0);
        encoder.uint64_value(1);
        encoder.uint64_value(2);
        encoder.uint64_value(3);
        encoder.end_array();
        encoder.end_object();

        check_encode_ubjson({'{','U',0x02,'o','c','[','U',0x00,'U',0x01,'U',0x02,'U',0x03,']','}'}, v);
    }
}

namespace { namespace ns {

    struct Person
    {
        std::string name;
    };

}}

JSONCONS_ALL_MEMBER_TRAITS(ns::Person, name)
    
TEST_CASE("encode_ubjson overloads")
{
    SECTION("json, stream")
    {
        jsoncons::json person;
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        ubjson::encode_ubjson(person, ss);
        jsoncons::json other = ubjson::decode_ubjson<jsoncons::json>(ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        ubjson::encode_ubjson(person, ss);
        ns::Person other = ubjson::decode_ubjson<ns::Person>(ss);
        CHECK(other.name == person.name);
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

TEST_CASE("encode_ubjson allocator_set overloads")
{
    MyScopedAllocator<char> temp_alloc(1);

    auto aset = make_alloc_set(jsoncons::temp_alloc_arg, temp_alloc);

    SECTION("json, stream")
    {
        jsoncons::json person;
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        ubjson::encode_ubjson(aset, person, ss);
        jsoncons::json other = ubjson::decode_ubjson<jsoncons::json>(aset, ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        ubjson::encode_ubjson(aset, person, ss);
        ns::Person other = ubjson::decode_ubjson<ns::Person>(aset, ss);
        CHECK(other.name == person.name);
    }
}

#endif
