// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/generic_to_json_visitor.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>
#include <catch/catch.hpp>

TEST_CASE("generic_to_json_visitor tests")
{
    SECTION("visit_string")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "key1" : "string",
    "key2" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.string_value("key1");
        adaptor.string_value("string");
        adaptor.string_value("key2");
        adaptor.int64_value(10);
        adaptor.end_object();

        REQUIRE(decoder.is_valid());
        std::cout << decoder.get_result() << "\n";
        //CHECK(expected == decoder.get_result());
    }
    /*SECTION("visit_bool")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "true" : "string",
    "false" : 10
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.string_value("true");
        adaptor.string_value("string");
        adaptor.string_value("false");
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.null_value();
        adaptor.string_value("string");
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.double_value(10.0);
        adaptor.string_value("string");
        adaptor.double_value(20.0);
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.int64_value(-20);
        adaptor.string_value("string");
        adaptor.int64_value(20);
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.uint64_value(10);
        adaptor.string_value("string");
        adaptor.uint64_value(20);
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        std::vector<uint8_t> purr = {'p','u','r','r'};
        adaptor.byte_string_value(purr);
        adaptor.string_value("string");
        std::vector<uint8_t> hiss = {'h','i','s','s'};
        adaptor.byte_string_value(hiss, jsoncons::semantic_tag::base64);
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.uint64_value(10);
        adaptor.string_value("string");
        adaptor.uint64_value(20);
        adaptor.int64_value(10);
        adaptor.end_object();

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
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.begin_object();
        adaptor.end_object();
        adaptor.string_value("string");
        adaptor.begin_array();
        adaptor.end_array();
        adaptor.int64_value(10);
        adaptor.end_object();

        REQUIRE(decoder.is_valid());
        CHECK(expected == decoder.get_result());

        //std::cout << decoder.get_result() << "\n";
    }
    SECTION("empty object")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "{}" : "string"
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.begin_object();
        adaptor.end_object();
        adaptor.string_value("string");
        adaptor.end_object();

        REQUIRE(decoder.is_valid());
        //CHECK(expected == decoder.get_result());

        std::cout << decoder.get_result() << "\n";
    }
    SECTION("empty array")
    {
        auto expected = jsoncons::json::parse(R"(
{
    "[]" : "string"
}
        )");

        jsoncons::json_decoder<jsoncons::json> decoder;
        jsoncons::generic_to_json_visitor2 adaptor(decoder);
        adaptor.begin_object();
        adaptor.begin_array();
        adaptor.end_array();
        adaptor.string_value("string");
        adaptor.end_object();

        REQUIRE(decoder.is_valid());
        //CHECK(expected == decoder.get_result());

        std::cout << decoder.get_result() << "\n";
    }*/
}

