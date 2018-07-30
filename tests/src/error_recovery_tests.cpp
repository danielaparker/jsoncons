
// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(error_recovery_tests)

class relaxed_error_handler : public parse_error_handler
{
private:

    bool do_error(std::error_code ec,
                  const serializing_context&) JSONCONS_NOEXCEPT override
    {
        if (ec == jsoncons::json_parse_errc::extra_comma)
        {
            return false;
        }
        return true;
    }
};

BOOST_AUTO_TEST_CASE(test_array_extra_comma)
{
    relaxed_error_handler err_handler;

    json expected = json::parse("[1,2,3]");
    json val = json::parse("[1,2,3,]", err_handler);

    BOOST_CHECK_EQUAL(expected,val);
}

BOOST_AUTO_TEST_CASE(test_object_extra_comma)
{
    relaxed_error_handler err_handler;

    json expected = json::parse(R"(
    {
        "first" : 1,
        "second" : 2
    }
    )", 
    err_handler);

    json val = json::parse(R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )", 
    err_handler);

    BOOST_CHECK_EQUAL(expected,val);
}

BOOST_AUTO_TEST_CASE(test_name_without_quotes)
{
    relaxed_error_handler err_handler;

    /*json val = json::parse(R"(
    {
        first : 1,
        second : 2
    }
    )", 
    err_handler);

    std::cout << val << std::endl;*/
}


BOOST_AUTO_TEST_SUITE_END()

