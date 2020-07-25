// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>

using namespace jsoncons;

TEST_CASE("json_const_pointer array tests")
{
    json j = json::parse(R"( ["one", "two", "three"] )");

    SECTION("size()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v.size() == 3);
        CHECK_FALSE(v.empty());
    }
    SECTION("at()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_array());
        REQUIRE_THROWS(v.at(1));
    }
    SECTION("at() const")
    {
        const json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v.at(1) == std::string("two"));
    }
    SECTION("copy")
    {
        json v(json_const_pointer_arg, &j);
        CHECK(v.storage() == storage_kind::json_const_pointer);

        json j2(v);
        CHECK(j2.storage() == storage_kind::array_value);
    }
    SECTION("assignment")
    {
        json v(json_const_pointer_arg, &j);
        CHECK(v.storage() == storage_kind::json_const_pointer);

        json j2;
        j2 = v;
        CHECK(j2.storage() == storage_kind::array_value);
    }
}

TEST_CASE("json_const_pointer object tests")
{
    json j = json::parse(R"( {"one" : 1, "two" : 2, "three" : 3} )");

    SECTION("size()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_object());
        CHECK(v.size() == 3);
        CHECK_FALSE(v.empty());
    }
    SECTION("at()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_object());
        REQUIRE_THROWS(v.at("two"));
        CHECK(v.contains("two"));
        CHECK(v.count("two") == 1);

        CHECK(v.get_value_or<int>("three", 0) == 3);
        CHECK(v.get_value_or<int>("four", 4) == 4);
    }
    SECTION("at() const")
    {
        const json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_object());
        CHECK(v.at("two") == 2);
        CHECK(v.contains("two"));
        CHECK(v.count("two") == 1);

        CHECK(v.get_value_or<int>("three", 0) == 3);
        CHECK(v.get_value_or<int>("four", 4) == 4);
    }
}

TEST_CASE("json_const_pointer string tests")
{
    json j = json("Hello World");

    SECTION("is_string()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_string());
        REQUIRE(v.is_string_view());

        CHECK(v.as<std::string>() == j.as<std::string>());
    }
}

TEST_CASE("json_const_pointer byte_string tests")
{
    std::string data = "abcdefghijk";
    json j(byte_string_arg, data);

    SECTION("is_byte_string()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_byte_string());
        REQUIRE(v.is_byte_string_view());
    }
}

TEST_CASE("json_const_pointer bool tests")
{
    json tru(true);
    json fal(false);

    SECTION("true")
    {
        json v(json_const_pointer_arg, &tru);
        REQUIRE(v.is_bool());
        CHECK(v.as_bool());
    }
    SECTION("false")
    {
        json v(json_const_pointer_arg, &fal);
        REQUIRE(v.is_bool());
        CHECK_FALSE(v.as_bool());
    }
}

TEST_CASE("json_const_pointer int64 tests")
{
    json j(-100);

    SECTION("is_int64()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_int64());
        CHECK(v.as<int64_t>() == -100);
    }
}

TEST_CASE("json_const_pointer uint64 tests")
{
    json j(100);

    SECTION("is_uint64()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_uint64());
        CHECK(v.as<uint64_t>() == 100);
    }
}

TEST_CASE("json_const_pointer half tests")
{
    json j(half_arg, 100);

    SECTION("is_half()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_half());
        CHECK(v.as<uint16_t>() == 100);
    }
}

TEST_CASE("json_const_pointer double tests")
{
    json j(123.456);

    SECTION("is_double()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_double());

        CHECK(v.as_double() == 123.456);
    }
}

