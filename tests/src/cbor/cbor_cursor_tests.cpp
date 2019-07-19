// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("cbor_cursor reputon test")
{
    ojson j = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> data;
    cbor::encode_cbor(j, data);

    SECTION("test 1")
    {
        cbor::cbor_bytes_cursor cursor(data);

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("cbor_cursor indefinite array of array test")
{
    std::vector<uint8_t> data = {0x82,0x83,0x63,0x66,0x6f,0x6f,0x44,0x50,0x75,0x73,0x73,0xc3,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0x63,0x62,0x61,0x72,0xd6,0x44,0x50,0x75,0x73,0x73,0xc4,0x82,0x21,0x19,0x6a,0xb3};

    SECTION("test 1")
    {
        cbor::cbor_bytes_cursor cursor(data);
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::byte_string_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::bigint);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::byte_string_value);
        CHECK(cursor.current().tag() == semantic_tag::base64);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::bigdec);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next();
        CHECK(cursor.done());
    }
}

