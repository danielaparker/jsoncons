// Copyright 2017 Daniel Parker
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
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(jsonpointer_tests)

void check_get_with_const_ref(const json& example, const std::string& pointer, const json& expected)
{

    std::error_code ec;
    const json& result = jsonpointer::get(example,pointer,ec);
    BOOST_CHECK(!ec);
    BOOST_CHECK_EQUAL(expected,result);
}

void check_contains(const json& example, const std::string& pointer, bool expected)
{
    bool result = jsonpointer::contains(example,pointer);
    BOOST_CHECK_EQUAL(expected,result);
}

void check_add(json& example, const std::string& path, const json& value, const json& expected)
{
    std::error_code ec;
    jsonpointer::insert_or_assign(example, path, value, ec);
    BOOST_CHECK(!ec);
    BOOST_CHECK_EQUAL(expected, example);
}

void check_replace(json& example, const std::string& path, const json& value, const json& expected)
{
    std::error_code ec;
    jsonpointer::replace(example, path, value, ec);
    BOOST_CHECK(!ec);
    BOOST_CHECK_EQUAL(expected, example);
}

void check_remove(json& example, const std::string& path, const json& expected)
{
    std::error_code ec;
    jsonpointer::remove(example, path, ec);
    BOOST_CHECK(!ec);
    BOOST_CHECK_EQUAL(expected, example);
}

BOOST_AUTO_TEST_CASE(get_with_const_ref_test)
{
// Example from RFC 6901
const json example = json::parse(R"(
   {
      "foo": ["bar", "baz"],
      "": 0,
      "a/b": 1,
      "c%d": 2,
      "e^f": 3,
      "g|h": 4,
      "i\\j": 5,
      "k\"l": 6,
      " ": 7,
      "m~n": 8
   }
)");

    check_contains(example,"",true);
    check_contains(example,"/foo",true);
    check_contains(example,"/foo/0",true);
    check_contains(example,"/",true);
    check_contains(example,"/a~1b",true);
    check_contains(example,"/c%d",true);
    check_contains(example,"/e^f",true);
    check_contains(example,"/g|h",true);
    check_contains(example,"/i\\j",true);
    check_contains(example,"/k\"l",true);
    check_contains(example,"/ ",true);
    check_contains(example,"/m~0n",true);

    check_get_with_const_ref(example,"",example);
    check_get_with_const_ref(example,"/foo",json::parse("[\"bar\", \"baz\"]"));
    check_get_with_const_ref(example,"/foo/0",json("bar"));
    check_get_with_const_ref(example,"/",json(0));
    check_get_with_const_ref(example,"/a~1b",json(1));
    check_get_with_const_ref(example,"/c%d",json(2));
    check_get_with_const_ref(example,"/e^f",json(3));
    check_get_with_const_ref(example,"/g|h",json(4));
    check_get_with_const_ref(example,"/i\\j",json(5));
    check_get_with_const_ref(example,"/k\"l",json(6));
    check_get_with_const_ref(example,"/ ",json(7));
    check_get_with_const_ref(example,"/m~0n",json(8));
}

BOOST_AUTO_TEST_CASE(get_with_ref_test)
{
// Example from RFC 6901
json example = json::parse(R"(
   {
      "foo": ["bar", "baz"]
   }
)");

    std::error_code ec;
    json& result = jsonpointer::get(example,"/foo/0",ec);
    BOOST_CHECK(!ec);

    result = "bat";

    std::cout << example << std::endl;
}

// add

BOOST_AUTO_TEST_CASE(test_add_object_member)
{
    json example = json::parse(R"(
    { "foo": "bar"}
    )");

    const json expected = json::parse(R"(
    { "foo": "bar", "baz" : "qux"}
    )");

    check_add(example,"/baz", json("qux"), expected);
}

BOOST_AUTO_TEST_CASE(test_add_array_element)
{
    json example = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    const json expected = json::parse(R"(
    { "foo": [ "bar", "qux", "baz" ] }
    )");

    check_add(example,"/foo/1", json("qux"), expected);
}

BOOST_AUTO_TEST_CASE(test_add_array_value)
{
    json example = json::parse(R"(
     { "foo": ["bar"] }
    )");

    const json expected = json::parse(R"(
    { "foo": ["bar", ["abc", "def"]] }
    )");

    check_add(example,"/foo/-", json::array({"abc", "def"}), expected);
}

// remove

BOOST_AUTO_TEST_CASE(test_remove_object_member)
{
    json example = json::parse(R"(
    { "foo": "bar", "baz" : "qux"}
    )");

    const json expected = json::parse(R"(
        { "foo": "bar"}
    )");

    check_remove(example,"/baz", expected);
}

BOOST_AUTO_TEST_CASE(test_remove_array_element)
{
    json example = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    const json expected = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    check_remove(example,"/foo/1", expected);
}

// replace

BOOST_AUTO_TEST_CASE(test_replace_object_value)
{
    json example = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    const json expected = json::parse(R"(
        {
          "baz": "boo",
          "foo": "bar"
        }
    )");

    check_replace(example,"/baz", json("boo"), expected);
}

BOOST_AUTO_TEST_CASE(test_replace_array_value)
{
    json example = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    const json expected = json::parse(R"(
        { "foo": [ "bar", "qux" ] }
    )");

    check_replace(example,"/foo/1", json("qux"), expected);
}

BOOST_AUTO_TEST_SUITE_END()




