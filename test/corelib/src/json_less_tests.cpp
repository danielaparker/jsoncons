// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("json null less")
{
    jsoncons::json j1 = jsoncons::null_type();

    SECTION("empty object")
    {
        jsoncons::json j2;

        CHECK(j1 < j2);
        CHECK_FALSE(j2 < j1);
    }

    SECTION("object")
    {
        jsoncons::json j2;
        j2["a"] = 1;
        j2["b"] = 3;
        j2["c"] = 3;

        CHECK(j1 < j2);
        CHECK_FALSE(j2 < j1);
    }
}

TEST_CASE("json empty object less")
{
    jsoncons::json j1;

    SECTION("empty object")
    {
        jsoncons::json j2;

        CHECK_FALSE(j1 < j2);
    }

    SECTION("object with no members")
    {
        jsoncons::json j2(jsoncons::json_object_arg);

        CHECK_FALSE(j1 < j2);
    }

    SECTION("object with members")
    {
        jsoncons::json j2;
        j2["a"] = 1;
        j2["b"] = 3;
        j2["c"] = 3;

        CHECK(j1 < j2);
    }
}

TEST_CASE("json bool less")
{
    jsoncons::json jtrue = true;
    jsoncons::json jfalse = false;

    SECTION("bool")
    {
        CHECK(jfalse < jtrue);
        CHECK_FALSE(jtrue < jfalse);
    }

    SECTION("null")
    {
        jsoncons::json j = jsoncons::null_type();
        CHECK(j < jfalse);
        CHECK(j < jtrue);
    }
}

TEST_CASE("json integer less")
{
    SECTION("-1 < 3")
    {
        jsoncons::json lhs(-1);
        jsoncons::json rhs(3);

        CHECK(lhs < rhs);
        CHECK(lhs <= rhs);
        CHECK_FALSE(rhs < lhs);
        CHECK_FALSE(rhs <= lhs);
    }
    SECTION("-1 < uint64_t(3)")
    {
        jsoncons::json lhs(-1);
        jsoncons::json rhs(uint64_t(3));

        CHECK(lhs < rhs);
        CHECK(lhs <= rhs);
        CHECK_FALSE(rhs < lhs);
        CHECK_FALSE(rhs <= lhs);
    }
}

TEST_CASE("json short string less")
{
    jsoncons::json j1 = "bcd";

    SECTION("short string")
    {
        jsoncons::json j2 = "cde";
        CHECK(j1 < j2);
        CHECK_FALSE(j2 < j1);
        jsoncons::json j3 = "bcda";
        CHECK(j1 < j3);
        CHECK_FALSE(j3 < j1);
    }

    SECTION("long string")
    {
        jsoncons::json j2 = "string too long for short string";
        CHECK(j1 < j2);
        CHECK_FALSE(j2 < j1);

        jsoncons::json j3 = "a string too long for short string";
        CHECK(j3 < j1);
        CHECK_FALSE(j1 < j3);
    }
}

TEST_CASE("json long string less")
{
    jsoncons::json j1 = "a string too long for short string";

    SECTION("short string")
    {
        jsoncons::json j2 = "a s";
        CHECK(j2 < j1);
        CHECK_FALSE(j1 < j2);
        jsoncons::json j3 = "bcd";
        CHECK(j1 < j3);
        CHECK_FALSE(j3 < j1);
    }

    SECTION("long string")
    {
        jsoncons::json j2 = "string too long for short string";
        CHECK(j1 < j2);
        CHECK_FALSE(j2 < j1);
    }
}

TEST_CASE("json array of string less")
{
    jsoncons::json j1(jsoncons::json_array_arg);
    j1.push_back("b");
    j1.push_back("c");
    j1.push_back("d");

    SECTION("array")
    {
        jsoncons::json j2(jsoncons::json_array_arg);
        j2.push_back("a");
        j2.push_back("b");
        j2.push_back("c");

        CHECK(j2 < j1);
        CHECK_FALSE(j1 < j2);
    }
}

