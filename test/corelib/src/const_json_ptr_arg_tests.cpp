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

TEST_CASE("const_json_ref array tests")
{
    jsoncons::json j = jsoncons::json::parse(R"( ["one", "two", "three"] )");

    SECTION("size()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(3 == v.size());
        CHECK_FALSE(v.empty());
    }
    SECTION("at()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_array());
        REQUIRE_THROWS(v.at(1));
    }
    SECTION("at() const")
    {
        const json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v.at(1) == std::string("two"));
    }
    SECTION("operator[]()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_array());
        REQUIRE_THROWS(v[1]);
    }
    SECTION("operator[]() const")
    {
        const json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v[1] == std::string("two"));
    }
    SECTION("copy")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::const_json_ref);

        jsoncons::json j2(v);
        CHECK(j2.storage_kind() == json_storage_kind::array);
    }
    SECTION("assignment")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        CHECK(v.storage_kind() == json_storage_kind::const_json_ref);

        jsoncons::json j2;
        j2 = v;
        CHECK(j2.storage_kind() == json_storage_kind::array);
    }
}

TEST_CASE("const_json_ref object tests")
{
    jsoncons::json j = jsoncons::json::parse(R"( {"one" : 1, "two" : 2, "three" : 3} )");

    SECTION("size()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_object());
        CHECK(3 == v.size());
        CHECK_FALSE(v.empty());
    }
    SECTION("at()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_object());
        REQUIRE_THROWS(v.at("two"));
        CHECK(v.contains("two"));
        CHECK(1 == v.count("two"));

        CHECK(3 == v.get_value_or<int>("three", 0));
        CHECK(4 == v.get_value_or<int>("four", 4));
    }
    SECTION("at() const")
    {
        const json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_object());
        CHECK(2 == v.at("two"));
        CHECK(v.contains("two"));
        CHECK(1 == v.count("two"));

        CHECK(3 == v.get_value_or<int>("three", 0));
        CHECK(4 == v.get_value_or<int>("four", 4));
    }
}

TEST_CASE("const_json_ref string tests")
{
    jsoncons::json j = jsoncons::json("Hello World");

    SECTION("is_string()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_string());
        REQUIRE(v.is_string_view());

        CHECK(v.as<std::string>() == j.as<std::string>());
    }
}

TEST_CASE("const_json_ref jsoncons::byte_string tests")
{
    std::string data = "abcdefghijk";
    jsoncons::json j(jsoncons::byte_string_arg, data);

    SECTION("is_byte_string()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_byte_string());
        REQUIRE(v.is_byte_string_view());
    }
}

TEST_CASE("const_json_ref bool tests")
{
    jsoncons::json tru(true);
    jsoncons::json fal(false);

    SECTION("true")
    {
        jsoncons::json v(const_json_ptr_arg, &tru);
        REQUIRE(v.is_bool());
        CHECK(v.as_bool());
    }
    SECTION("false")
    {
        jsoncons::json v(const_json_ptr_arg, &fal);
        REQUIRE(v.is_bool());
        CHECK_FALSE(v.as_bool());
    }
}

TEST_CASE("const_json_ref int64 tests")
{
    jsoncons::json j(-100);

    SECTION("is_int64()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_int64());
        CHECK(v.as<int64_t>() == -100);
    }
}

TEST_CASE("const_json_ref uint64 tests")
{
    jsoncons::json j(100);

    SECTION("is_uint64()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_uint64());
        CHECK(v.as<uint64_t>() == 100);
    }
}

TEST_CASE("const_json_ref half tests")
{
    jsoncons::json j(half_arg, 100);

    SECTION("is_half()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_half());
        CHECK(v.as<uint16_t>() == 100);
    }
}

TEST_CASE("const_json_ref double tests")
{
    jsoncons::json j(123.456);

    SECTION("is_double()")
    {
        jsoncons::json v(const_json_ptr_arg, &j);
        REQUIRE(v.is_double());

        CHECK(v.as_double() == 123.456);
    }
}

namespace {

    void flatten(const jsoncons::json& source, 
                 const std::string& identifier, 
                 jsoncons::json& result)
    {
        jsoncons::json temp(jsoncons::json_array_arg);
        for (auto& item : source.array_range())
        {
            if (item.is_array())
            {
                for (auto& item_of_item : item.array_range())
                {
                    temp.emplace_back(const_json_ptr_arg, &item_of_item);
                }
            }
            else
            {
                temp.emplace_back(const_json_ptr_arg, &item);
            }
        }
        for (const auto& item : temp.array_range())
        {
            if (!item.is_null())
            {
                const auto& j = item.contains(identifier) ? item.at(identifier) : jsoncons::json::null();
                if (!j.is_null())
                {
                    result.emplace_back(const_json_ptr_arg, &j);
                }
            }
        }
    }
}

TEST_CASE("const_json_ref identifier tests")
{
    jsoncons::json source = jsoncons::json::parse(R"(
    {"reservations": [{
        "instances": [
            {"foo": [{"bar": 1}, {"bar": 2}, {"notbar": 3}, {"bar": 4}]},
            {"foo": [{"bar": 5}, {"bar": 6}, {"notbar": [7]}, {"bar": 8}]},
            {"foo": "bar"},
            {"notfoo": [{"bar": 20}, {"bar": 21}, {"notbar": [7]}, {"bar": 22}]},
            {"bar": [{"baz": [1]}, {"baz": [2]}, {"baz": [3]}, {"baz": [4]}]},
            {"baz": [{"baz": [1, 2]}, {"baz": []}, {"baz": []}, {"baz": [3, 4]}]},
            {"qux": [{"baz": []}, {"baz": [1, 2, 3]}, {"baz": [4]}, {"baz": []}]}
        ],
        "otherkey": {"foo": [{"bar": 1}, {"bar": 2}, {"notbar": 3}, {"bar": 4}]}
      }, {
        "instances": [
            {"a": [{"bar": 1}, {"bar": 2}, {"notbar": 3}, {"bar": 4}]},
            {"b": [{"bar": 5}, {"bar": 6}, {"notbar": [7]}, {"bar": 8}]},
            {"c": "bar"},
            {"notfoo": [{"bar": 23}, {"bar": 24}, {"notbar": [7]}, {"bar": 25}]},
            {"qux": [{"baz": []}, {"baz": [1, 2, 3]}, {"baz": [4]}, {"baz": []}]}
        ],
        "otherkey": {"foo": [{"bar": 1}, {"bar": 2}, {"notbar": 3}, {"bar": 4}]}
      }
    ]}
    )");

    SECTION("test1")
    {
        jsoncons::json target;
        jsoncons::json j1(jsoncons::json_array_arg);
        jsoncons::json j2(jsoncons::json_array_arg);
        jsoncons::json j3(jsoncons::json_array_arg);
        jsoncons::json expected = jsoncons::json::parse("[1,2,4,5,6,8]");
        const json v1(const_json_ptr_arg, &source.at("reservations"));
        flatten(v1, "instances", j1);

        const json v2(const_json_ptr_arg, &j1);
        flatten(v2, "foo", j2);

        const json v3(const_json_ptr_arg, &j2);
        flatten(v3, "bar", j3);

        target = j3;
        CHECK(expected == target);
    }

    SECTION("test2")
    {
        jsoncons::json expected = jsoncons::json::parse("[1,2,4,5,6,8]");
        jsoncons::json target;
        {
            jsoncons::json j1(jsoncons::json_array_arg);
            jsoncons::json j2(jsoncons::json_array_arg);
            jsoncons::json j3(jsoncons::json_array_arg);
            const json v1(const_json_ptr_arg, &source.at("reservations"));
            flatten(v1, "instances", j1);

            const json v2(const_json_ptr_arg, &j1);
            flatten(v2, "foo", j2);

            const json v3(const_json_ptr_arg, &j2);
            flatten(v3, "bar", j3);

            target = jsoncons::json(j3);
        }
        CHECK(expected == target);
        CHECK(target.storage_kind() == json_storage_kind::array);
        for (const auto& item : target.array_range())
        {
            CHECK(item.storage_kind() == json_storage_kind::uint64);
        }
    }
}

