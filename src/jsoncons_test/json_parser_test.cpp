// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "my_custom_data.hpp"

using jsoncons::parsing_context;
using jsoncons::json_deserializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_listener;
using jsoncons::error_handler;
using jsoncons::json_parse_exception;
using std::string;

class my_error_handler : public error_handler
{

public:
    my_error_handler(std::string error_code, 
                     std::string fatal_error_code)
        : error_code_(error_code), 
          fatal_error_code_(fatal_error_code)
    {
    }

    virtual void warning(const std::string& error_code, 
                         const std::string& message, 
                         const parsing_context& context) throw(json_parse_exception)
    {
    }

    virtual void error(const std::string& error_code, 
                       const std::string& message, 
                       const parsing_context& context) throw(json_parse_exception)
    {
        BOOST_CHECK(error_code == error_code_);
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }

    virtual void fatal_error(const std::string& error_code, 
                             const std::string& message, 
                             const parsing_context& context) throw(json_parse_exception)
    {
        if (error_code != fatal_error_code_)
        {
            std::cout << error_code << " " << message << std::endl;
        }
        BOOST_CHECK(error_code == fatal_error_code_);
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }

    std::string error_code_;
    std::string fatal_error_code_;
};

BOOST_AUTO_TEST_CASE(test_missing_separator)
{
    std::istringstream is("{\"field1\"{}}");

    json_deserializer handler;
    my_error_handler err_handler("","JPE106");

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_invalid_value)
{
    std::istringstream is("{\"field1\":ru}");

    json_deserializer handler;
    my_error_handler err_handler("","JPE105");

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_unexpected_end_of_file)
{
    std::istringstream is("{\"field1\":{}");

    json_deserializer handler;
    my_error_handler err_handler("","JPE101");

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}

BOOST_AUTO_TEST_CASE(test_value_not_found)
{
    std::istringstream is("{\"field1\":}");

    json_deserializer handler;
    my_error_handler err_handler("","JPE204");

    json_reader reader(is,handler,err_handler);

    BOOST_REQUIRE_THROW(reader.read(), json_parse_exception);
}


