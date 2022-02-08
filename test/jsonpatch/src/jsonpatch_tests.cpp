// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using jsoncons::json;
namespace jsonpatch = jsoncons::jsonpatch;
using namespace jsoncons::literals;

void check_patch(json& target, const json& patch, const std::error_code& expected_ec, const json& expected)
{
    std::error_code ec;
    jsonpatch::apply_patch(target, patch, ec);
    if (ec != expected_ec || expected != target)
    {
        std::cout << "target:\n" << target << std::endl;
    }
    CHECK(ec == expected_ec); //-V521
    CHECK(target == expected); //-V521
}

TEST_CASE("testing_a_value_success")
{
    json target = R"(
        {
            "baz": "qux",
            "foo": [ "a", 2, "c" ]
        }
    )"_json;

    json patch = R"(
        [
           { "op": "test", "path": "/baz", "value": "qux" },
           { "op": "test", "path": "/foo/1", "value": 2 }
        ]
    )"_json;

    json expected = target;

    check_patch(target,patch,std::error_code(),expected);
}

TEST_CASE("testing_a_value_error")
{
    json target = R"(
        { "baz": "qux" }

    )"_json;

    json patch = R"(
        [
           { "op": "test", "path": "/baz", "value": "bar" }
        ]
    )"_json;

    json expected = target;

    check_patch(target,patch,jsonpatch::jsonpatch_errc::test_failed,expected);
}

TEST_CASE("comparing_strings_and_numbers")
{
    json target = R"(
        {
            "/": 9,
            "~1": 10
        }

    )"_json;

    json patch = R"(
        [
            {"op": "test", "path": "/~01", "value": "10"}
        ]
    )"_json;

    json expected = target;

    check_patch(target,patch,jsonpatch::jsonpatch_errc::test_failed,expected);
}

TEST_CASE("test_add_add")
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] }
        ]
    )"_json;

    json expected = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    check_patch(target,patch,std::error_code(),expected);
}

TEST_CASE("test_diff1")
{
    json source = R"(
        {"/": 9, "~1": 10, "foo": "bar"}
    )"_json;

    json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    auto patch = jsonpatch::from_diff(source, target);

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("test_diff2")
{
    json source = R"(
        { 
            "/": 3,
            "foo": "bar"
        }
    )"_json;

    json target = R"(
        {
            "/": 9,
            "~1": 10
        }
    )"_json;

    auto patch = jsonpatch::from_diff(source, target);

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("add_when_new_items_in_target_array1")
{
    json source = R"(
        {"/": 9, "foo": [ "bar"]}
    )"_json;

    json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("add_when_new_items_in_target_array2")
{
    json source = R"(
        {"/": 9, "foo": [ "bar", "bar"]}
    )"_json;

    json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("jsonpatch - remove two items from array")
{
    json source = json::parse(R"(
{ "names" : [ "a", "b", "c", "d" ] }
    )");

    json target = json::parse(R"(
{ "names" : [ "a", "b" ] }
    )");

    json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}



