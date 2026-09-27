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

using namespace jsoncons;

TEST_CASE("json_ref array tests")
{
    jsoncons::json j = jsoncons::json::parse(R"( [1, "two", "three"] )");

    SECTION("size()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v.get_allocator() == j.get_allocator());
        REQUIRE(j.size() == v.size());
        v.resize(4);
        REQUIRE(4 == v.size());
        CHECK(jsoncons::json{} == v[3]);

        v.resize(5, jsoncons::json{jsoncons::null_arg});
        REQUIRE(5 == v.size());
        CHECK(jsoncons::json{} == v[3]);
        CHECK(jsoncons::json::null() == v[4]);
        CHECK(v[4].is_null());
    }

    SECTION("compare with const_json_ptr_arg")
    {
        jsoncons::json other{ j };
        jsoncons::json j1(jsoncons::json_ptr_arg, &other);
        jsoncons::json j2(const_json_ptr_arg, &other);
        
        CHECK(j1 == j2);
        CHECK(j == j1);
        CHECK(j == j2);

        j[0] = "one";

        CHECK(j1 == j2);
        CHECK_FALSE(j == j1);
        CHECK_FALSE(j == j2);
    }
    SECTION("capacity()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(j.capacity() == v.capacity());
        v.reserve(4);
        REQUIRE(4 == v.capacity());
    }

    SECTION("empty()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK_FALSE(v.empty());
    }

    SECTION("is_int64()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v[0].is_int64());
        CHECK_FALSE(v[1].is_int64());
    }
    SECTION("is_number()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v[0].is_number());
        CHECK_FALSE(v[1].is_number());
    }
    SECTION("operator[]")
    {
        jsoncons::json expected = jsoncons::json::parse(R"( [1, "two", "four"] )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);
        j[2] = "four";

        CHECK(expected == v);
    }
    SECTION("const operator[]")
    {
        const json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);

        CHECK("three" == v[2]);
    }
    SECTION("at()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        REQUIRE_NOTHROW(v.at(1));
        CHECK("two" == v[1]);
    }
    SECTION("const at()")
    {
        const json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_array());
        REQUIRE_NOTHROW(v.at(1));
        CHECK("two" == v[1]);
    }
    SECTION("copy")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);

        jsoncons::json j2(v);
        CHECK(j2.storage_kind() == json_storage_kind::array);
    }
    SECTION("assignment")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);

        jsoncons::json j2;
        j2 = v;
        CHECK(j2.storage_kind() == json_storage_kind::array);
    }
    SECTION("push_back")
    {
        jsoncons::json expected = jsoncons::json::parse(R"( [1, "two", "three", "four"] )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);
        j.push_back("four");
        
        CHECK(expected == v);
    }
    SECTION("emplace_back")
    {
        jsoncons::json expected = jsoncons::json::parse(R"( [1, "two", "three", "four"] )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::json_ref);
        j.emplace_back("four");

        CHECK(expected == v);
    }
}

TEST_CASE("json_ref object tests")
{
    jsoncons::json j = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : 3} )");

    SECTION("size()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());
        CHECK(3 == v.size());
        CHECK_FALSE(v.empty());
    }

    SECTION("compare with const_json_ptr_arg")
    {
        jsoncons::json other{ j };
        jsoncons::json j1(jsoncons::json_ptr_arg, &other);
        jsoncons::json j2(const_json_ptr_arg, &other);

        CHECK(j1 == j2);
        CHECK(j == j1);
        CHECK(j == j2);

        j["one"] = 4;

        CHECK(j1 == j2);
        CHECK_FALSE(j == j1);
        CHECK_FALSE(j == j2);
    }
    SECTION("at()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());
        REQUIRE_NOTHROW(v.at("two"));
        CHECK(v.contains("two"));
        CHECK(1 == v.count("two"));

        CHECK(3 == v.get_value_or<int>("three", 0));
        CHECK(4 == v.get_value_or<int>("four", 4));
        
        v.at("one") = "first";
        CHECK("first" == v.at("one"));
    }
    SECTION("insert_or_assign()")
    {
        jsoncons::json expected = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : "third", "four" : 4} )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());
        REQUIRE_NOTHROW(v.at("two"));
        CHECK(v.contains("two"));
        CHECK(1 == v.count("two"));

        CHECK(3 == v.get_value_or<int>("three", 0));
        CHECK(4 == v.get_value_or<int>("four", 4));

        v.insert_or_assign("four", 4);
        v.insert_or_assign("three", "third");
        CHECK(expected == v);
    }
    SECTION("try_emplace()")
    {
        jsoncons::json expected = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : 3, "four" : 4} )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());
        REQUIRE_NOTHROW(v.at("two"));
        CHECK(v.contains("two"));
        CHECK(1 == v.count("two"));

        CHECK(3 == v.get_value_or<int>("three", 0));
        CHECK(4 == v.get_value_or<int>("four", 4));

        v.try_emplace("four", 4);
        v.try_emplace("three", "third"); // does nothing
        CHECK(expected == v);
    }
    SECTION("merge()")
    {
        jsoncons::json expected1 = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : 3, "four" : 4} )");
        jsoncons::json expected2 = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : 3, "four" : 4, "five" : 5} )");
        
        jsoncons::json j1 = jsoncons::json::parse(R"( {"three" : "third", "four" : 4} )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());

        v.merge(j1);
        CHECK(expected1 == v);

        jsoncons::json j2 = jsoncons::json::parse(R"( {"five" : 5} )");
        j2.merge(v);
        CHECK(expected2 == j2);
    }
    SECTION("merge_or_update()")
    {
        jsoncons::json expected1 = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : "third", "four" : 4} )");
        jsoncons::json expected2 = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : "third", "four" : 4, "five" : 5} )");

        jsoncons::json j1 = jsoncons::json::parse(R"( {"three" : "third", "four" : 4} )");

        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_object());

        v.merge_or_update(j1);
        CHECK(expected1 == v);

        jsoncons::json j2 = jsoncons::json::parse(R"( {"five" : 5} )");
        j2.merge_or_update(v);
        CHECK(expected2 == j2);
    }
}

TEST_CASE("json_ref string tests")
{
    jsoncons::json j = jsoncons::json("Hello World");

    SECTION("is_string()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_string());
        REQUIRE(v.is_string_view());

        CHECK(v.as<std::string>() == j.as<std::string>());
    }
}

TEST_CASE("json_ref jsoncons::byte_string tests")
{
    std::string data = "abcdefghijk";
    jsoncons::json j(jsoncons::byte_string_arg, data);

    SECTION("is_byte_string()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_byte_string());
        REQUIRE(v.is_byte_string_view());
    }
}

TEST_CASE("json_ref bool tests")
{
    jsoncons::json tru(true);
    jsoncons::json fal(false);

    SECTION("true")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &tru);
        REQUIRE(v.is_bool());
        CHECK(v.as_bool());
    }
    SECTION("false")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &fal);
        REQUIRE(v.is_bool());
        CHECK_FALSE(v.as_bool());
    }
}

TEST_CASE("json_ref null tests")
{
    jsoncons::json null(jsoncons::null_arg);

    SECTION("null")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &null);
        REQUIRE(v.is_null());
    }
}

TEST_CASE("json_ref int64 tests")
{
    jsoncons::json j(-100);

    SECTION("is_int64()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_int64());
        CHECK(v.as<int64_t>() == -100);
    }
}

TEST_CASE("json_ref uint64 tests")
{
    jsoncons::json j(100);

    SECTION("is_uint64()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_uint64());
        CHECK(v.as<uint64_t>() == 100);
    }
}

TEST_CASE("json_ref half tests")
{
    jsoncons::json j(half_arg, 100);

    SECTION("is_half()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_half());
        CHECK(v.as<uint16_t>() == 100);
    }
}

TEST_CASE("json_ref double tests")
{
    jsoncons::json j(123.456);

    SECTION("is_double()")
    {
        jsoncons::json v(jsoncons::json_ptr_arg, &j);
        REQUIRE(v.is_double());

        CHECK(v.as_double() == 123.456);
    }
}

