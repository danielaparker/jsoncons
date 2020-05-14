// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

TEST_CASE("json constructor byte_string_arg tests")
{
    std::string expected_base64url = "Zm9vYmFy";

    SECTION("byte_string_arg std::vector<uint8_t>")
    {
        std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};
        json j(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(j.as<std::string>() == expected_base64url);
    }
    SECTION("byte_string_arg std::string")
    {
        std::string bytes = {'f','o','o','b','a','r'};
        json j(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(j.as<std::string>() == expected_base64url);
    }
}

TEST_CASE("json constructor tests")
{
    SECTION("json json_object_arg")
    {
        json j1(json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(j1.size() == 1);
        CHECK(j1.at("one").as<int>() == 1);

        json j2(json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(j2.size() == 2);
        CHECK(j2.at("one").as<int>() == 1);
        CHECK(j2.at("two").as<int>() == 2);
    }
    SECTION("json json_array_arg")
    {
        json j1(json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(j1.size() == 1);
        CHECK(j1[0].as<int>() == 1);

        json j2(json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(j2.size() == 2);
        CHECK(j2[0].as<int>() == 1);
        CHECK(j2[1].as<int>() == 2);
    }
    SECTION("ojson json_object_arg")
    {
        ojson j1(json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(j1.size() == 1);
        CHECK(j1.at("one").as<int>() == 1);

        ojson j2(json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(j2.size() == 2);
        CHECK(j2.at("one").as<int>() == 1);
        CHECK(j2.at("two").as<int>() == 2);
    }
    SECTION("ojson json_array_arg")
    {
        ojson j1(json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(j1.size() == 1);
        CHECK(j1[0].as<int>() == 1);

        ojson j2(json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(j2.size() == 2);
        CHECK(j2[0].as<int>() == 1);
        CHECK(j2[1].as<int>() == 2);
    }
}
TEST_CASE("json(string_view)")
{
    json::string_view_type sv("Hello world.");

    json j(sv);

    CHECK(j.as<json::string_view_type>() == sv);
    CHECK(j.as_string_view() == sv);
}

TEST_CASE("json(string, semantic_tag::datetime)")
{
    std::string s("2015-05-07 12:41:07-07:00");

    json j(s, semantic_tag::datetime);

    CHECK(j.tag() == semantic_tag::datetime);
    CHECK(j.as<std::string>() == s);
}


TEST_CASE("json(string, semantic_tag::timestamp)")
{
    SECTION("positive integer")
    {
        int t = 10000;
        json j(t, semantic_tag::timestamp);

        CHECK(j.tag() == semantic_tag::timestamp);
        CHECK(j.as<int>() == t);
    }
    SECTION("negative integer")
    {
        int t = -10000;
        json j(t, semantic_tag::timestamp);

        CHECK(j.tag() == semantic_tag::timestamp);
        CHECK(j.as<int>() == t);
    }
    SECTION("floating point")
    {
        double t = 10000.1;
        json j(t, semantic_tag::timestamp);

        CHECK(j.tag() == semantic_tag::timestamp);
        CHECK(j.as<double>() == t);
    }

}

TEST_CASE("json get_allocator() tests")
{
    SECTION("short string")
    {
        json j("short");

        CHECK(j.get_allocator() == json::allocator_type());
    }
    SECTION("long string")
    {
        json::allocator_type alloc;
        json j("string too long for short string", alloc);

        CHECK(j.get_allocator() == alloc);
    }
    SECTION("byte string")
    {
        json::allocator_type alloc;
        json j(byte_string({'H','e','l','l','o'}),alloc);

        CHECK(j.get_allocator() == alloc);
    }
    SECTION("array")
    {
        json::allocator_type alloc;
        json j(json_array_arg, semantic_tag::none, alloc);

        REQUIRE(j.is_array());
        CHECK(j.get_allocator() == alloc);
    }
    SECTION("object")
    {
        json::allocator_type alloc;
        json j(json_object_arg, semantic_tag::none, alloc);

        REQUIRE(j.is_object());
        CHECK(j.get_allocator() == alloc);
    }
}

