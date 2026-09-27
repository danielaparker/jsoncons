// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <jsoncons/json.hpp>

#include <iostream>
#include <catch/catch.hpp>

using jsoncons::json;
using jsoncons::json_options;
using jsoncons::ojson;
namespace jsonpatch = jsoncons::jsonpatch;
using namespace jsoncons::literals;

template <typename Json>
void check_patch(Json& target, const Json& patch, const std::error_code& expected_ec, const Json& expected)
{
    std::error_code ec;
    jsonpatch::apply_patch(target, patch, ec);
    if (ec != expected_ec || expected != target)
    {
        std::cout << "target:\n" << target << '\n';
    }
    CHECK(ec == expected_ec); //-V521
    CHECK(expected == target); //-V521
}

TEST_CASE("testing_a_value_success")
{
    jsoncons::json target = R"(
        {
            "baz": "qux",
            "foo": [ "a", 2, "c" ]
        }
    )"_json;

    jsoncons::json patch = R"(
        [
           { "op": "test", "path": "/baz", "value": "qux" },
           { "op": "test", "path": "/foo/1", "value": 2 }
        ]
    )"_json;

    jsoncons::json expected = target;

    check_patch(target,patch,std::error_code(),expected);
}

TEST_CASE("testing_a_value_error")
{
    jsoncons::json target = R"(
        { "baz": "qux" }

    )"_json;

    jsoncons::json patch = R"(
        [
           { "op": "test", "path": "/baz", "value": "bar" }
        ]
    )"_json;

    jsoncons::json expected = target;

    check_patch(target,patch,jsoncons::jsonpatch::jsonpatch_errc::test_failed,expected);
}

TEST_CASE("comparing_strings_and_numbers")
{
    jsoncons::json target = R"(
        {
            "/": 9,
            "~1": 10
        }

    )"_json;

    jsoncons::json patch = R"(
        [
            {"op": "test", "path": "/~01", "value": "10"}
        ]
    )"_json;

    jsoncons::json expected = target;

    check_patch(target,patch,jsoncons::jsonpatch::jsonpatch_errc::test_failed,expected);
}

TEST_CASE("test_add_add")
{
    jsoncons::json target = R"(
        { "foo": "bar"}
    )"_json;

    jsoncons::json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] }
        ]
    )"_json;

    jsoncons::json expected = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    check_patch(target,patch,std::error_code(),expected);
}

TEST_CASE("test_diff1")
{
    jsoncons::json source = R"(
        {"/": 9, "~1": 10, "foo": "bar"}
    )"_json;

    jsoncons::json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    auto patch = jsonpatch::from_diff(source, target);

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("test_diff2")
{
    jsoncons::json source = R"(
        { 
            "/": 3,
            "foo": "bar"
        }
    )"_json;

    jsoncons::json target = R"(
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
    jsoncons::json source = R"(
        {"/": 9, "foo": [ "bar"]}
    )"_json;

    jsoncons::json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    jsoncons::json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("add_when_new_items_in_target_array2")
{
    jsoncons::json source = R"(
        {"/": 9, "foo": [ "bar", "bar"]}
    )"_json;

    jsoncons::json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    jsoncons::json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("jsonpatch - remove two items from array")
{
    jsoncons::json source = jsoncons::json::parse(R"(
{ "names" : [ "a", "b", "c", "d" ] }
    )");

    jsoncons::json target = jsoncons::json::parse(R"(
{ "names" : [ "a", "b" ] }
    )");

    jsoncons::json patch = jsoncons::jsonpatch::from_diff(source, target); 

    check_patch(source,patch,std::error_code(),target);
}

TEST_CASE("from diff with null and lossless number")
{
    jsoncons::ojson expected_patch = jsoncons::ojson::parse(
        R"([{"op":"replace","path":"/hello","value":null},{"op":"replace","path":"/hello2","value":"123.4"}])"
    );
    
    auto options = jsoncons::json_options{}
        .lossless_number(true)
        .bignum_format(jsoncons::bignum_format_kind::raw)
        .byte_string_format(jsoncons::byte_string_chars_format::base64);

    const char* json1 = "{\"hello\":123.4, \"hello2\":null}";
    const char* json2 = "{\"hello\":null,  \"hello2\":123.4 }";

    jsoncons::ojson j1 = jsoncons::ojson::parse(jsoncons::json1, options);
    jsoncons::ojson j2 = jsoncons::ojson::parse(jsoncons::json2, options);

    jsoncons::ojson patch = jsonpatch::from_diff(j1, j2);
    
    CHECK(expected_patch == patch);
    check_patch(j1,patch,std::error_code(),j2);
}

TEST_CASE("replace_root_with_object_via_add")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "add", "path" : "", "value": {} }])");
    jsoncons::json expected = jsoncons::json::parse(R"({})");
    check_patch(target, patch, std::error_code(), expected);
}

TEST_CASE("replace_root_with_object_via_replace")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "replace", "path" : "", "value": {} }])");
    jsoncons::json expected = jsoncons::json::parse(R"({})");
    check_patch(target, patch, std::error_code(), expected);
}

TEST_CASE("remove_root")
{
    jsoncons::json target = jsoncons::json::parse(R"({})");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "remove", "path" : "" }])");
    jsoncons::json expected = target;
    check_patch(target, patch, jsonpatch::jsonpatch_errc::remove_failed, expected);
}

TEST_CASE("test_root")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "test", "path" : "", "value": { "child" : [ "a", "b", "c", "d" ] } }])");
    jsoncons::json expected = target;
    check_patch(target, patch, std::error_code(), expected);
}

TEST_CASE("move_child_to_root")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "move", "path" : "", "from": "/child" }])");
    jsoncons::json expected = jsoncons::json::parse(R"([ "a", "b", "c", "d" ])");
    check_patch(target, patch, std::error_code(), expected);
}

TEST_CASE("move_root_to_child")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "move", "path" : "/child", "from": "" }])");
    jsoncons::json expected = target;
    check_patch(target, patch, jsonpatch::jsonpatch_errc::move_failed, expected);
}

TEST_CASE("copy_root_to_child")
{
    jsoncons::json target = jsoncons::json::parse(R"({ "child" : [ "a", "b", "c", "d" ] })");
    jsoncons::json patch = jsoncons::json::parse(R"([{ "op" : "copy", "path" : "/child_copy", "from": "" }])");
    jsoncons::json expected = jsoncons::json::parse(R"(
        { "child" : [ "a", "b", "c", "d" ],
          "child_copy" : { "child" : [ "a", "b", "c", "d" ] } }
    )");
    check_patch(target, patch, std::error_code(), expected);
}
