// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>
#include <catch/catch.hpp>

TEST_CASE("json decoder key tests")
{
    SECTION("visit_key")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "key1" : "string",
    "key2" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.key("key1");
        decoder.string_value("string");
        decoder.string_value("key2");
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("visit_string")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "key1" : "string",
    "key2" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.string_value("key1");
        decoder.string_value("string");
        decoder.string_value("key2");
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("visit_bool")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "true" : "string",
    "false" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.string_value("true");
        decoder.string_value("string");
        decoder.string_value("false");
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }

    SECTION("visit_null")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "null" : "string"
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.null_value();
        decoder.string_value("string");
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
}

