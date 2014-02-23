// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons/json_deserializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_deserializer;
using jsoncons::json;
using jsoncons::json_reader;
using jsoncons::pretty_print;
using std::string;
using jsoncons::json_exception;
using jsoncons::json_parse_exception;

BOOST_AUTO_TEST_CASE(test_filename_invalid)
{
    std::string in_file = "input/json-exception--1.json";
    std::ifstream is(in_file);

    json_deserializer handler;

    try
    {
        json_reader reader(is,handler);
        reader.read();
    }
    catch (const json_exception&)
    {
    }
}

BOOST_AUTO_TEST_CASE(test_exception_left_brace)
{
    std::string in_file = "input/json-exception-1.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        BOOST_CHECK_EQUAL(e.line_number(), 14);
        BOOST_CHECK_EQUAL(e.column_number(), 30);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_right_brace)
{
    std::string in_file = "input/json-exception-2.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        BOOST_CHECK_EQUAL(e.line_number(),17);
        BOOST_CHECK_EQUAL(e.column_number(), 9);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_array_eof)
{
    std::istringstream is("[100");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 4);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_unicode_eof)
{
    std::istringstream is("[\"\\u");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 3);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_tru_eof)
{
    std::istringstream is("[tru");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 2);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_fals_eof)
{
    std::istringstream is("[fals");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 2);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_nul_eof)
{
    std::istringstream is("[nul");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 2);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_true_eof)
{
    std::istringstream is("[true");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 5);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_false_eof)
{
    std::istringstream is("[false");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 6);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_null_eof)
{
    std::istringstream is("[null");

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const json_parse_exception& e)
    {
        std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(e.line_number(),1);
        BOOST_CHECK_EQUAL(e.column_number(), 5);
    }
}

