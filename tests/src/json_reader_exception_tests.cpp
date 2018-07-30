// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_decoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_reader_exception_tests)

BOOST_AUTO_TEST_CASE(test_filename_invalid)
{
    std::string in_file = "./input/json-exception--1.json";
    std::ifstream is(in_file);

    json_decoder<json> decoder;

    try
    {
        json_reader reader(is,decoder);
        reader.read_next();
    }
    catch (const std::exception&)
    {
    }
    //BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_left_brace)
{
    std::string in_file = "./input/json-exception-1.json";
    std::ifstream is(in_file);

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();
    }
    catch (const parse_error& e)
    {
        BOOST_CHECK(e.code() == json_parse_errc::expected_comma_or_right_bracket);
        BOOST_CHECK_EQUAL(14,e.line_number());
        BOOST_CHECK_EQUAL(30,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}
BOOST_AUTO_TEST_CASE(test_exception_right_brace)
{
    std::string in_file = "./input/json-exception-2.json";
    std::ifstream is(in_file);

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK(e.code() == json_parse_errc::expected_comma_or_right_brace);
        BOOST_CHECK_EQUAL(17,e.line_number());
        BOOST_CHECK_EQUAL(9, e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_array_eof)
{
    std::istringstream is("[100");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_unicode_eof)
{
    std::istringstream is("[\"\\u");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_tru_eof)
{
    std::istringstream is("[tru");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_fals_eof)
{
    std::istringstream is("[fals");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_nul_eof)
{
    std::istringstream is("[nul");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        //std::cout << e.what() << std::endl;
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(5,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_true_eof)
{
    std::istringstream is("[true");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_false_eof)
{
    std::istringstream is("[false");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(7,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception_null_eof)
{
    std::istringstream is("[null");

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();  // must throw
        BOOST_CHECK(0 != 0);
    }
    catch (const parse_error& e)
    {
        BOOST_CHECK(e.code() == json_parse_errc::unexpected_eof);
        BOOST_CHECK_EQUAL(1,e.line_number());
        BOOST_CHECK_EQUAL(6,e.column_number());
    }
    BOOST_CHECK_EQUAL(false,decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_exception)
{
    std::string input("{\"field1\":\n\"value}");
    BOOST_CHECK_EXCEPTION(json::parse(input),
                          parse_error,
                          [](const parse_error& e)
                            {return e.code() == json_parse_errc::unexpected_eof && e.line_number() == 2 && e.column_number() == 9;}
                         );
}

BOOST_AUTO_TEST_SUITE_END()
