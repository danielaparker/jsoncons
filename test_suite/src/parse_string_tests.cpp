// Copyright 2016 Daniel Parker
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
using jsoncons::json_text_error_category;
using jsoncons::json_exception;
using jsoncons::default_parse_error_handler;

struct jsonpath_filter_fixture
{
};

class lenient_error_handler : public parse_error_handler
{
public:
	lenient_error_handler(int value)
		: value_(value)
	{
	}
private:
    int value_;

    virtual void do_warning(std::error_code ec,
                            const parsing_context& context) throw(json_parse_exception)
    {
    }

    virtual void do_error(std::error_code ec,
                          const parsing_context& context) throw(json_parse_exception)
    {
        if (ec.category() == json_text_error_category())
        {
            if (ec.value() != value_)
            {
                default_parse_error_handler::instance().error(ec,context);
            }
        }
    }
};

BOOST_AUTO_TEST_CASE(test_parse_small_string1)
{
    std::string input = "\"String\"";
    std::istringstream is(input);

    json_deserializer handler;
    try
    {
        json_reader reader(is,handler);
        reader.read_next();
    }
    catch (const json_exception&)
    {
    }
    BOOST_CHECK(handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_parse_small_string2)
{
	std::string input = "\"Str\\\"ing\"";
	std::istringstream is(input);

	json_deserializer handler;
	try
	{
		json_reader reader(is, handler);
		reader.read_next();
	}
	catch (const json_exception&)
	{
	}
	BOOST_CHECK(handler.is_valid());
}

BOOST_AUTO_TEST_CASE(test_parse_small_string4)
{
	std::string input = "\"Str\\\"ing\"";

    for (size_t i = 2; i < input.length(); ++i)
    {
        std::istringstream is(input);
        json_deserializer handler;
        try
        {
            json_reader reader(is, handler);
            reader.buffer_capacity(i);
            reader.read_next();
        }
        catch (const json_exception&)
        {
        }
        BOOST_CHECK(handler.is_valid());
        BOOST_CHECK_EQUAL("Str\"ing",handler.get_result().as_cstring());
    }
}
BOOST_AUTO_TEST_CASE(test_parse_big_string1)
{
	std::string input = "\"Big Str\\\"ing\"";

    for (size_t i = 2; i < input.length(); ++i)
    {
        std::istringstream is(input);
        json_deserializer handler;
        try
        {
            json_reader reader(is, handler);
            reader.buffer_capacity(i);
            reader.read_next();
        }
        catch (const json_exception&)
        {
        }
        BOOST_CHECK(handler.is_valid());
        BOOST_CHECK_EQUAL("Big Str\"ing",handler.get_result().as_cstring());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_big_string2)
{
	std::string input = "\"Big\t Str\\\"ing\"";

    //for (size_t i = 2; i < input.length(); ++i)
    //{
        std::istringstream is(input);
        json_deserializer handler;
        lenient_error_handler err_handler(jsoncons::json_parser_errc::illegal_character_in_string);
        try
        {
            json_reader reader(is, handler, err_handler);
            //reader.buffer_capacity(i);
            reader.read_next();
        }
        catch (const json_exception&)
        {
        }
        BOOST_CHECK(handler.is_valid());
        BOOST_CHECK_EQUAL("Big\t Str\"ing",handler.get_result().as_cstring());
    //}
}


