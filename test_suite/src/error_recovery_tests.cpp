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
using jsoncons::input_error_handler;
using jsoncons::json_parse_exception;
using jsoncons::json_parser_category;
using std::string;

class error_recovery_handler : public input_error_handler
{

public:
    error_recovery_handler(int error_code)
        : error_code_(error_code)
    {
    }

private:
    virtual void do_warning(std::error_code ec,
                            parsing_context context) throw(json_parse_exception)
    {
    }

    virtual void do_error(std::error_code ec,
                          parsing_context context) throw(json_parse_exception)
    {
        BOOST_CHECK(ec.category() == json_parser_category());
        BOOST_CHECK(ec.value() == error_code_);
        throw json_parse_exception(ec,context.line_number(),context.column_number());
    }

    int error_code_;
};

BOOST_AUTO_TEST_CASE(test_missing_name_quote)
{
}


