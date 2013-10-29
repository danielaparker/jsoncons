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
using jsoncons::output_format;
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
    }
    catch (const json_parse_exception& e)
    {
        std::cout << "***" << e.what() << "," << e.line_number() << "," << e.column_number() << std::endl;
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
        reader.read();
    }
    catch (const json_parse_exception& e)
    {
        std::cout << "***" << e.what() << "," << e.line_number() << "," << e.column_number() << std::endl;
    }
} 

