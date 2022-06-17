// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_cursor2.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("cbor_cursor2 reputon test")
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
        cbor::cbor_bytes_cursor2 cursor(data);

        CHECK(cursor.current().event_type() == staj2_event_type::begin_object);
        CHECK(cursor.current().size() == 2);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::begin_array);
        CHECK(cursor.current().size() == 1);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_object);
        cursor.next();
        CHECK(cursor.done());
    }
}

