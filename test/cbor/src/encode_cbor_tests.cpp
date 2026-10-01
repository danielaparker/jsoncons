// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons/json.hpp>
#include <iostream>
#include <vector>
#include <catch/catch.hpp>

namespace cbor = jsoncons::cbor;

// test vectors from tinycbor https://github.com/01org/tinycbor tst_encoder.cpp
// MIT license

void check_encode_cbor(const std::vector<uint8_t>& expected, const jsoncons::json& j)
{
    std::vector<uint8_t> result;
    cbor::encode_cbor(j,result);

    if (result.size() != expected.size())
    {
        std::cout << j << "\n";
    }
    REQUIRE(result.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        REQUIRE(result[i] == expected[i]);
    }
}

TEST_CASE("cbor_encoder_test")
{
    // unsigned integer
    check_encode_cbor({0x00},jsoncons::json(0U));
    check_encode_cbor({0x01},jsoncons::json(1U));
    check_encode_cbor({0x0a},jsoncons::json(10U));
    check_encode_cbor({0x17},jsoncons::json(23U));
    check_encode_cbor({0x18,0x18},jsoncons::json(24U));
    check_encode_cbor({0x18,0xff},jsoncons::json(255U));
    check_encode_cbor({0x19,0x01,0x00},jsoncons::json(256U));
    check_encode_cbor({0x19,0xff,0xff},jsoncons::json(65535U));
    check_encode_cbor({0x1a,0,1,0x00,0x00},jsoncons::json(65536U));
    check_encode_cbor({0x1a,0xff,0xff,0xff,0xff},jsoncons::json(4294967295U));
    check_encode_cbor({0x1b,0,0,0,1,0,0,0,0},jsoncons::json(4294967296U));
    check_encode_cbor({0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<uint64_t>::max)()));

    // positive signed integer
    check_encode_cbor({0x00},jsoncons::json(0));
    check_encode_cbor({0x01},jsoncons::json(1));
    check_encode_cbor({0x0a},jsoncons::json(10));
    check_encode_cbor({0x17},jsoncons::json(23));
    check_encode_cbor({0x18,0x18},jsoncons::json(24));
    check_encode_cbor({0x18,0xff},jsoncons::json(255));
    check_encode_cbor({0x19,0x01,0x00},jsoncons::json(256));
    check_encode_cbor({0x19,0xff,0xff},jsoncons::json(65535));
    check_encode_cbor({0x1a,0,1,0x00,0x00},jsoncons::json(65536));
    check_encode_cbor({0x1a,0xff,0xff,0xff,0xff},jsoncons::json(4294967295));
    check_encode_cbor({0x1b,0,0,0,1,0,0,0,0},jsoncons::json(4294967296));
    check_encode_cbor({0x1b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<int64_t>::max)()));

    // negative integers
    check_encode_cbor({0x20},jsoncons::json(-1));
    check_encode_cbor({0x21},jsoncons::json(-2));
    check_encode_cbor({0x37},jsoncons::json(-24));
    check_encode_cbor({0x38,0x18},jsoncons::json(-25));
    check_encode_cbor({0x38,0xff},jsoncons::json(-256));
    check_encode_cbor({0x39,0x01,0x00},jsoncons::json(-257));
    check_encode_cbor({0x39,0xff,0xff},jsoncons::json(-65536));
    check_encode_cbor({0x3a,0,1,0x00,0x00},jsoncons::json(-65537));
    check_encode_cbor({0x3a,0xff,0xff,0xff,0xff},jsoncons::json(-4294967296));
    check_encode_cbor({0x3b,0,0,0,1,0,0,0,0},jsoncons::json(-4294967297));

    // null, true, false
    check_encode_cbor({0xf6},jsoncons::json::null());
    check_encode_cbor({0xf5},jsoncons::json(true));
    check_encode_cbor({0xf4},jsoncons::json(false));

    // floating point
    check_encode_cbor({0xfa,0,0,0,0},jsoncons::json(0.0));
    check_encode_cbor({0xfa,0xbf,0x80,0,0},jsoncons::json(-1.0));

    SECTION("-16777215.0")
    {
        double val = -16777215.0;
        float valf = (float)val;
        CHECK((double)valf == val);
        check_encode_cbor({0xfa,0xcb,0x7f,0xff,0xff},
                          jsoncons::json(val));
    }
    // From https://en.wikipedia.org/wiki/Double-precision_floating-point_forma    
    SECTION("0.333333333333333314829616256247390992939472198486328125")
    {
        double val = 0.333333333333333314829616256247390992939472198486328125;
        float valf = (float)val;
        CHECK((double)valf != val);
        check_encode_cbor({0xfb,0x3F,0xD5,0x55,0x55,0x55,0x55,0x55,0x55},jsoncons::json(val));
    }

    // byte string
    check_encode_cbor({0x40},jsoncons::json(jsoncons::byte_string()));
    check_encode_cbor({0x41,' '},jsoncons::json(jsoncons::byte_string({' '})));
    check_encode_cbor({0x41,0},jsoncons::json(jsoncons::byte_string({0})));
    check_encode_cbor({0x45,'H','e','l','l','o'},jsoncons::json(jsoncons::byte_string({'H','e','l','l','o'})));
    check_encode_cbor({0x58,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
        jsoncons::json(jsoncons::byte_string({ '1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4' })));

    // text string
    check_encode_cbor({0x60},jsoncons::json(""));
    check_encode_cbor({0x61,' '},jsoncons::json(" "));
    check_encode_cbor({0x78,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 jsoncons::json("123456789012345678901234"));

}

TEST_CASE("cbor_arrays_and_maps")
{
    check_encode_cbor({ 0x80 }, jsoncons::json(jsoncons::json_array_arg));
    check_encode_cbor({ 0xa0 }, jsoncons::json());

    check_encode_cbor({ 0x81,'\0' }, jsoncons::json::parse("[0]"));
    check_encode_cbor({ 0x82,'\0','\0' }, jsoncons::json(jsoncons::json_array_arg, { 0,0 }));
    check_encode_cbor({ 0x82,0x81,'\0','\0' }, jsoncons::json::parse("[[0],0]"));
    check_encode_cbor({ 0x81,0x65,'H','e','l','l','o' }, jsoncons::json::parse("[\"Hello\"]"));

    // big float
    jsoncons::json j("0x6AB3p-2", jsoncons::semantic_tag::bigfloat);
    CHECK(j.tag() == jsoncons::semantic_tag::bigfloat);
    jsoncons::json j2 = j;
    CHECK(j2.tag() == jsoncons::semantic_tag::bigfloat);
    jsoncons::json j3;
    j3 = j;
    CHECK(j3.tag() == jsoncons::semantic_tag::bigfloat);

    check_encode_cbor({ 0xc5, // Tag 5 
                         0x82, // Array of length 2
                           0x21, // -2 
                             0x19, 0x6a, 0xb3 // 27315 
        }, jsoncons::json("0x6AB3p-2", jsoncons::semantic_tag::bigfloat));

    check_encode_cbor({ 0xa1,0x62,'o','c',0x81,'\0' }, jsoncons::json::parse("{\"oc\": [0]}"));
    check_encode_cbor({ 0xa1,0x62,'o','c',0x84,'\0','\1','\2','\3' }, jsoncons::json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

namespace { namespace ns {

    struct Person
    {
        std::string name;
    };

}}

JSONCONS_ALL_MEMBER_TRAITS(ns::Person, name)

TEST_CASE("encode_cbor overloads")
{
    SECTION("json, stream")
    {
        jsoncons::json person;
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(person, ss);
        jsoncons::json other = cbor::decode_cbor<jsoncons::json>(ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(person, ss);
        ns::Person other = cbor::decode_cbor<ns::Person>(ss);
        CHECK(other.name == person.name);
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using cust_json = jsoncons::basic_json<char,jsoncons::sorted_policy,MyScopedAllocator<char>>;

#if 0
TEST_CASE("encode_cbor allocator_set")
{
    MyScopedAllocator<char> result_alloc(1);
    MyScopedAllocator<char> temp_alloc(2);

    auto aset = make_alloc_set(result_alloc, temp_alloc);

    SECTION("json, stream")
    {
        cust_json person(jsoncons::json_object_arg, result_alloc);
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(aset, person, ss);
        cust_json other = cbor::decode_cbor<cust_json>(aset,ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(aset, person, ss);
        ns::Person other = cbor::decode_cbor<ns::Person>(aset,ss);
        CHECK(other.name == person.name);
    }
}
#endif

TEST_CASE("encode_cbor allocator_set for temp only")
{
    MyScopedAllocator<char> temp_alloc(1);

    auto aset = make_alloc_set(jsoncons::temp_alloc_arg, temp_alloc);

    SECTION("json, stream")
    {
        jsoncons::json person;
        person.try_emplace("name", "John Smith");

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(aset, person, ss);
        auto other = cbor::decode_cbor<jsoncons::json>(aset,ss);
        CHECK(other == person);
    }
    SECTION("custom, stream")
    {
        ns::Person person{"John Smith"};

        std::string s;
        std::stringstream ss(s);
        cbor::encode_cbor(aset, person, ss);
        ns::Person other = cbor::decode_cbor<ns::Person>(aset,ss);
        CHECK(other.name == person.name);
    }
}

#endif
