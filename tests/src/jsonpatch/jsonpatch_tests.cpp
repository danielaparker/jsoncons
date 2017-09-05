// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <codecvt>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

BOOST_AUTO_TEST_SUITE(jsonpatch_tests)

void check_good_patch(json& target, const json& patch, const json& expected)
{
    jsonpatch::jsonpatch_errc ec;
    std::string op;
    std::string path;
    std::tie(ec,op,path) = jsonpatch::patch(target,patch);
    if (ec != jsonpatch::jsonpatch_errc())
    {
        std::cout << "op: " << op << std::endl;
        std::cout << "path: " << path << std::endl;
    }
    BOOST_CHECK(ec == jsonpatch::jsonpatch_errc());
    BOOST_CHECK_EQUAL(expected, target);
}

void check_bad_patch(json& target, const json& patch, jsonpatch::jsonpatch_errc expected_ec, const json& expected)
{
    jsonpatch::jsonpatch_errc ec;
    std::string op;
    std::string path;
    std::tie(ec,op,path) = jsonpatch::patch(target,patch);
    if (ec == jsonpatch::jsonpatch_errc())
    {
        std::cout << "op: " << op << std::endl;
        std::cout << "path: " << path << std::endl;
    }
    BOOST_CHECK(ec == expected_ec);
    BOOST_CHECK_EQUAL(expected, target);
}

BOOST_AUTO_TEST_CASE(add_an_object_member)
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
    [
        { "op": "add", "path": "/baz", "value": "qux" }
    ]
    )"_json;

    json expected = R"(
        {"baz":"qux","foo":"bar"}
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(add_an_array_element)
{
    json target = R"(
        { "foo": [ "bar", "baz" ] }
    )"_json;

    json patch = R"(
    [
        { "op": "add", "path": "/foo/1", "value": "qux" }
    ]
    )"_json;

    json expected = R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(remove_an_object_member)
{
    json target = R"(
        {
            "baz": "qux",
            "foo": "bar"
        }
    )"_json;

    json patch = R"(
    [
        { "op": "remove", "path": "/baz" }
    ]
    )"_json;

    json expected = R"(
        { "foo": "bar" }
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(remove_an_array_element)
{
    json target = R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )"_json;

    json patch = R"(
    [
        { "op": "remove", "path": "/foo/1" }
    ]
    )"_json;

    json expected = R"(
        { "foo": [ "bar", "baz" ] }
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(replace_a_value)
{
    json target = R"(
        {
            "baz": "qux",
            "foo": "bar"
        }
    )"_json;

    json patch = R"(
        [
            { "op": "replace", "path": "/baz", "value": "boo" }
        ]
    )"_json;

    json expected = R"(
        {
            "baz": "boo",
            "foo": "bar"
        }
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(move_a_value)
{
    json target = R"(
        {
            "foo": {
                "bar": "baz",
                "waldo": "fred"
            },
            "qux": {
                "corge": "grault"
            }
        }
    )"_json;

    json patch = R"(
    [
        { "op": "move", "from": "/foo/waldo", "path": "/qux/thud" }
    ]
    )"_json;

    json expected = R"(
       {
           "foo": {
                   "bar": "baz"
                  },
           "qux": {
                     "corge": "grault",
                     "thud": "fred"
                  }
       }
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(move_an_array_element)
{
    json target = R"(
        { "foo": [ "all", "grass", "cows", "eat" ] }
    )"_json;

    json patch = R"(
        [
           { "op": "move", "from": "/foo/1", "path": "/foo/3" }
        ]
    )"_json;

    json expected = R"(
        { "foo": [ "all", "cows", "eat", "grass" ] }    
    )"_json;

    check_good_patch(target,patch,expected);
}

BOOST_AUTO_TEST_CASE(add_to_nonexistent_target)
{
    json target = R"(
        { "foo": "bar" }
    )"_json;

    json patch = R"(
        [
           { "op": "add", "path": "/baz/bat", "value": "qux" }
        ]
    )"_json;

    json expected = target;

    check_bad_patch(target,patch,jsonpatch::jsonpatch_errc::add_failed,expected);
}

BOOST_AUTO_TEST_SUITE_END()




