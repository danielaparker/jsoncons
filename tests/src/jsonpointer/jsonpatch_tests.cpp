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
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(jsonpatch_tests)

void check_add(json& example, const std::string pointer, const json& value, const json& expected)
{
    try
    {
        jsonpointer::add(example, pointer, value);
        BOOST_CHECK_EQUAL(expected, example);
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << ". " << pointer << std::endl;
    }
}

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

BOOST_AUTO_TEST_SUITE_END()




