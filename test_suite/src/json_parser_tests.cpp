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
using jsoncons::error_handler;
using jsoncons::default_error_handler_impl;
using jsoncons::json_parse_exception;
using jsoncons::json_parser_category;
using std::string;

class my_error_handler : public error_handler
{

public:
    my_error_handler(int error_code)
        : error_code_(error_code)
    {
    }
    int error_code_;

private:
    virtual void do_warning(std::error_code ec,
                            const parsing_context& context) throw(json_parse_exception)
    {
    }

    virtual void do_error(std::error_code ec,
                          const parsing_context& context) throw(json_parse_exception)
    {
		error_code_ = ec.value();
        throw json_parse_exception(ec,context.line_number(),context.column_number());
    }

};

BOOST_AUTO_TEST_CASE(test_missing_separator)
{
    std::istringstream is("{\"field1\"{}}");

    json_deserializer handler;
    my_error_handler err_handler(jsoncons::json_parser_error::expected_name_separator);

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_invalid_value)
{
    std::istringstream is("{\"field1\":ru}");

    json_deserializer handler;
    my_error_handler err_handler(jsoncons::json_parser_error::expected_name_or_value);

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_unexpected_end_of_file)
{
    std::istringstream is("{\"field1\":{}");

    json_deserializer handler;
	my_error_handler err_handler(jsoncons::json_parser_error::unexpected_eof);

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_value_not_found)
{
    std::istringstream is("{\"field1\":}");

    json_deserializer handler;
    my_error_handler err_handler(jsoncons::json_parser_error::value_not_found);

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_escaped_characters)
{
    std::string input("[\"\\n\\b\\f\\r\\t\"]");
    std::string expected("\n\b\f\r\t");

    json o = json::parse_string(input);
    BOOST_CHECK(expected == o[0].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_expected_name)
{
	std::cout << "test_expected_name" << std::endl;
    std::istringstream is("{10}");
    
    my_error_handler err_handler(jsoncons::json_parser_error::expected_name);
	json_reader reader(is,jsoncons::null_json_input_handler<char>(),err_handler);

	try
	{
		reader.read();
	}
	catch (const json_parse_exception& e)
	{
		std::cout << e.what() << std::endl;
		BOOST_CHECK(err_handler.error_code_ == jsoncons::json_parser_error::expected_name);
	}

    //std::cout << in << std::endl;
}



