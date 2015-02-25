// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::parsing_context;
using jsoncons::json_deserializer;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_input_handler;
using jsoncons::parse_error_handler;
using jsoncons::json_parse_exception;
using jsoncons::json_parser_category;
using std::string;

void test_error_code(const std::string& text, int ec)
{
    std::istringstream is(text);
	json_reader reader(is,jsoncons::empty_json_input_handler::instance());
	try
	{
		reader.read();
		BOOST_FAIL("Must throw");
	}
	catch (const json_parse_exception& e)
	{
		BOOST_CHECK_MESSAGE(e.code().value() == ec, e.what());
	}
}

BOOST_AUTO_TEST_CASE(test_missing_separator)
{
    test_error_code("{\"field1\"{}}", jsoncons::json_parser_errc::expected_name_separator);
}

BOOST_AUTO_TEST_CASE(test_invalid_value)
{
    test_error_code("{\"field1\":ru}",jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_unexpected_end_of_file)
{
    test_error_code("{\"field1\":{}", jsoncons::json_parser_errc::unexpected_eof);
}

BOOST_AUTO_TEST_CASE(test_value_not_found)
{
    test_error_code("{\"name\":}", jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_escaped_characters)
{
    std::string input("[\"\\n\\b\\f\\r\\t\"]");
    std::string expected("\n\b\f\r\t");

    json o = json::parse_string(input);
    BOOST_CHECK(expected == o[0].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_expected_name_separator)
{
	test_error_code("{\"name\" 10}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" true}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" false}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" null}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" \"value\"}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" {}}", jsoncons::json_parser_errc::expected_name_separator);
	test_error_code("{\"name\" []}", jsoncons::json_parser_errc::expected_name_separator);
}

BOOST_AUTO_TEST_CASE(test_expected_name)
{
	test_error_code("{10}", jsoncons::json_parser_errc::expected_name);
	test_error_code("{true}", jsoncons::json_parser_errc::expected_name);
	test_error_code("{false}", jsoncons::json_parser_errc::expected_name);
	test_error_code("{null}", jsoncons::json_parser_errc::expected_name);
	test_error_code("{{}}", jsoncons::json_parser_errc::expected_name);
	test_error_code("{[]}", jsoncons::json_parser_errc::expected_name);
}

BOOST_AUTO_TEST_CASE(test_expected_value)
{
	test_error_code("[tru]", jsoncons::json_parser_errc::expected_value);
    test_error_code("[fa]", jsoncons::json_parser_errc::expected_value);
    test_error_code("[n]", jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_expected_container)
{
    test_error_code("null", jsoncons::json_parser_errc::expected_container);
    test_error_code("false", jsoncons::json_parser_errc::expected_container);
    test_error_code("true", jsoncons::json_parser_errc::expected_container);
    test_error_code("10", jsoncons::json_parser_errc::expected_container);
    test_error_code("\"string\"", jsoncons::json_parser_errc::expected_container);
}



