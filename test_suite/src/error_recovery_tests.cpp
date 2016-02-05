
// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(error_recovery_test_suite)

class my_parse_error_handler : public parse_error_handler
{
private:
    virtual void do_warning(std::error_code ec,
                            const parsing_context& context) throw(parse_exception)
    {
    }

    virtual void do_error(std::error_code ec,
                          const parsing_context& context) throw(parse_exception)
    {
        if (ec.category() == json_error_category())
        {
            if (ec.value() != jsoncons::json_parser_errc::extra_comma && (context.current_char() == ']' || context.current_char() == '}'))
            {
                default_parse_error_handler::instance().error(ec,context);
            }
        }
    }
};

BOOST_AUTO_TEST_CASE(test_accept_trailing_value_separator)
{
    /*my_parse_error_handler err_handler;

    json val = json::parse("[1,2,3,]", err_handler);

    std::cout << val << std::endl;*/
}


BOOST_AUTO_TEST_SUITE_END()

