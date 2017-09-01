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

BOOST_AUTO_TEST_SUITE(jsonpointer_tests)

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

void check_jsonpointer(const std::string pointer, const json& expected)
{
    try
    {
        json result = jsonpointer::select(example,pointer);
        BOOST_CHECK_EQUAL(expected,result);
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << ". " << pointer << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpointer)
{
    check_jsonpointer("",example);
    check_jsonpointer("/foo",json::parse("[\"bar\", \"baz\"]"));
    check_jsonpointer("/foo/0",json("bar"));
    check_jsonpointer("/",json(0));
    check_jsonpointer("/a~1b",json(1));
    check_jsonpointer("/c%d",json(2));
    check_jsonpointer("/e^f",json(3));
    check_jsonpointer("/g|h",json(4));
    check_jsonpointer("/i\\j",json(5));
    check_jsonpointer("/k\"l",json(6));
    check_jsonpointer("/ ",json(7));
    check_jsonpointer("/m~0n",json(8));
}

BOOST_AUTO_TEST_SUITE_END()




