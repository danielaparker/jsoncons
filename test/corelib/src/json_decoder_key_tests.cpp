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
    SECTION("visit_double")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "10.0" : "string",
    "20.0" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.double_value(10.0);
        decoder.string_value("string");
        decoder.double_value(20.0);
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("visit_int64")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "-20" : "string",
    "20" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.int64_value(-20);
        decoder.string_value("string");
        decoder.int64_value(20);
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("visit_uint64")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "10" : "string",
    "20" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.uint64_value(10);
        decoder.string_value("string");
        decoder.uint64_value(20);
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("visit_byte_string")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "cHVycg" : "string",
    "aGlzcw==" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        std::vector<uint8_t> purr = {'p','u','r','r'};
        decoder.byte_string_value(purr);
        decoder.string_value("string");
        std::vector<uint8_t> hiss = {'h','i','s','s'};
        decoder.byte_string_value(hiss, jsoncons::semantic_tag::base64);
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());

        //std::cout << decoder.get_result() << "\n";
    }
    SECTION("visit_uint64")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "10" : "string",
    "20" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.uint64_value(10);
        decoder.string_value("string");
        decoder.uint64_value(20);
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());
    }
    SECTION("empty object, empty array")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "{}" : "string",
    "[]" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        decoder.begin_object();
        decoder.begin_object();
        decoder.end_object();
        decoder.string_value("string");
        decoder.begin_array();
        decoder.end_array();
        decoder.int64_value(10);
        decoder.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());

        //std::cout << decoder.get_result() << "\n";
    }
}

