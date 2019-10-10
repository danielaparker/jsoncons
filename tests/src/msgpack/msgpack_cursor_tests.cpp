// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/msgpack/msgpack_cursor.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("msgpack_cursor reputon test")
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
    msgpack::encode_msgpack(j, data);

    SECTION("test 1")
    {
        msgpack::msgpack_bytes_cursor cursor(data);

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
struct remove_mark_msgpack_filter
{
    bool reject_next_ = false;

    bool operator()(const staj_event& event, const ser_context&) 
    {
        if (event.event_type()  == staj_event_type::name &&
            event.get<jsoncons::string_view>() == "mark")
        {
            reject_next_ = true;
            return false;
        }
        else if (reject_next_)
        {
            reject_next_ = false;
            return false;
        }
        else
        {
            return true;
        }
    }
};


TEST_CASE("msgpack_cursor with filter tests")
{
    auto j = ojson::parse(R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60
        }
    ]
    )");

    std::vector<uint8_t> data;
    msgpack::encode_msgpack(j, data);

    msgpack::msgpack_bytes_cursor cursor(data, remove_mark_msgpack_filter());

    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_array);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_array);
    cursor.next();
    CHECK(cursor.done());
}

