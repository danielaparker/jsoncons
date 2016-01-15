// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons/json_deserializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_reader_exception_test_suite)

BOOST_AUTO_TEST_CASE(test_filename_invalid)
{
    std::string in_file = "input/json-exception--1.json";
    std::ifstream is(in_file);

    json_deserializer handler;

    try
    {
        json_reader reader(is,handler);
        reader.read_next();
    }
    catch (const json_exception&)
    {
    }
    //BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_left_brace)
{
    std::string in_file = "input/json-exception-1.json";
    std::ifstream is(in_file);

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::expected_comma_or_right_bracket,e.code().value());
        BOOST_CHECK_EQUAL(14,e.line_number());
        BOOST_CHECK_EQUAL(30,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}
BOOST_AUTO_TEST_CASE(test_exception_right_brace)
{
    std::string in_file = "input/json-exception-2.json";
    std::ifstream is(in_file);

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(json_parser_errc::expected_comma_or_right_brace,e.code().value());
        BOOST_CHECK_EQUAL(17,e.line_number());
        BOOST_CHECK_EQUAL(9, e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_array_eof)
{
    std::istringstream is("[100");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_unicode_eof)
{
    std::istringstream is("[\"\\u");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_tru_eof)
{
    std::istringstream is("[tru");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_fals_eof)
{
    std::istringstream is("[fals");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_nul_eof)
{
    std::istringstream is("[nul");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_true_eof)
{
    std::istringstream is("[true");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_false_eof)
{
    std::istringstream is("[false");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(7,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_null_eof)
{
    std::istringstream is("[null");

	json_deserializer handler;
	try
    {
        json_reader reader(is,handler);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception)
{
    try
    {
        std::string input("{\"field1\":\n\"value}");
        std::cout << input << std::endl;
        json obj = json::parse(input);
    }
    catch (const parse_exception& e)
    {
        BOOST_CHECK_EQUAL(json_parser_errc::unexpected_eof,e.code().value());
        BOOST_CHECK_EQUAL(2,e.line_number());
        BOOST_CHECK_EQUAL(9,e.column_number());
    }
}

BOOST_AUTO_TEST_SUITE_END()
