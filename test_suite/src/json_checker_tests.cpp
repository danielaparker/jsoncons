// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons/json_deserializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

using namespace boost::filesystem;

BOOST_AUTO_TEST_SUITE(json_checker_test_suite)

BOOST_AUTO_TEST_CASE(test_fail1)
{
    std::string in_file = "input/JSON_checker/fail1.json";

    BOOST_CHECK_NO_THROW(json::parse_file(in_file));
}

BOOST_AUTO_TEST_CASE(test_fail2)
{
    std::string in_file = "input/JSON_checker/fail2.json";

    int err = 0;

    try
    {
        //json::parse_file(in_file);
        json val;
        std::ifstream is(in_file);
        is >> val;
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::unexpected_eof);
}

BOOST_AUTO_TEST_CASE(test_fail3)
{
    std::string in_file = "input/JSON_checker/fail3.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_name);
}

BOOST_AUTO_TEST_CASE(test_fail4)
{
    std::string in_file = "input/JSON_checker/fail4.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::extra_comma);
}

BOOST_AUTO_TEST_CASE(test_fail5)
{
    std::string in_file = "input/JSON_checker/fail5.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_fail6)
{
    std::string in_file = "input/JSON_checker/fail6.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_fail7)
{
    std::string in_file = "input/JSON_checker/fail7.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::extra_character);
}

BOOST_AUTO_TEST_CASE(test_fail8)
{
    std::string in_file = "input/JSON_checker/fail8.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::extra_character);
}

BOOST_AUTO_TEST_CASE(test_fail9)
{
    std::string in_file = "input/JSON_checker/fail9.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::extra_comma);
}

BOOST_AUTO_TEST_CASE(test_fail10)
{
    std::string in_file = "input/JSON_checker/fail10.json";

    int err = 0;
    
    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::extra_character);
}

BOOST_AUTO_TEST_CASE(test_fail11)
{
    std::string in_file = "input/JSON_checker/fail11.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_comma_or_right_brace);
}

BOOST_AUTO_TEST_CASE(test_fail12)
{
    std::string in_file = "input/JSON_checker/fail12.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_fail13)
{
    std::string in_file = "input/JSON_checker/fail13.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::leading_zero);
}

BOOST_AUTO_TEST_CASE(test_fail14)
{
    std::string in_file = "input/JSON_checker/fail14.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::invalid_number);
}

BOOST_AUTO_TEST_CASE(test_fail15)
{
    std::string in_file = "input/JSON_checker/fail15.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_escaped_character);
}

BOOST_AUTO_TEST_CASE(test_fail16)
{
    std::string in_file = "input/JSON_checker/fail16.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_fail17)
{
    std::string in_file = "input/JSON_checker/fail17.json";
    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_escaped_character);
}

BOOST_AUTO_TEST_CASE(test_fail18)
{
    std::string in_file = "input/JSON_checker/fail18.json";
    std::ifstream is(in_file);
    int err = 0;

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.max_nesting_depth(20);
        reader.read_next();
        reader.check_done();
    }
    catch (const parse_exception& e)
    {
         err = e.code().value();
         //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::max_depth_exceeded);
}

BOOST_AUTO_TEST_CASE(test_fail19)
{
    std::string in_file = "input/JSON_checker/fail19.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_colon);
}

BOOST_AUTO_TEST_CASE(test_fail20)
{
    std::string in_file = "input/JSON_checker/fail20.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_fail21)
{
    std::string in_file = "input/JSON_checker/fail21.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_colon);
}

BOOST_AUTO_TEST_CASE(test_fail22)
{
    std::string in_file = "input/JSON_checker/fail22.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_comma_or_right_bracket);
}

BOOST_AUTO_TEST_CASE(test_fail23)
{
    std::string in_file = "input/JSON_checker/fail23.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::invalid_value);
}

BOOST_AUTO_TEST_CASE(test_fail24)
{
    std::string in_file = "input/JSON_checker/fail24.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    // Single quote
    BOOST_CHECK(err == jsoncons::json_parser_errc::single_quote);
}

BOOST_AUTO_TEST_CASE(test_fail25)
{
    std::string in_file = "input/JSON_checker/fail25.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_character_in_string);
}

BOOST_AUTO_TEST_CASE(test_fail26)
{
    std::string in_file = "input/JSON_checker/fail26.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_escaped_character);
}

BOOST_AUTO_TEST_CASE(test_fail27)
{
    std::string in_file = "input/JSON_checker/fail27.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_character_in_string);
}

BOOST_AUTO_TEST_CASE(test_fail28)
{
    std::string in_file = "input/JSON_checker/fail28.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::illegal_escaped_character);
}

BOOST_AUTO_TEST_CASE(test_fail29)
{
    std::string in_file = "input/JSON_checker/fail29.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::invalid_number);
}

BOOST_AUTO_TEST_CASE(test_fail30)
{
    std::string in_file = "input/JSON_checker/fail30.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::invalid_number);
}

BOOST_AUTO_TEST_CASE(test_fail31)
{
    std::string in_file = "input/JSON_checker/fail31.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::invalid_number);
}

BOOST_AUTO_TEST_CASE(test_fail32)
{
    std::string in_file = "input/JSON_checker/fail32.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::unexpected_eof);
}

BOOST_AUTO_TEST_CASE(test_fail33)
{
    std::string in_file = "input/JSON_checker/fail33.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        err = e.code().value();
        //std::cout << in_file << " " << e.what() << std::endl;
    }
    BOOST_CHECK(err == jsoncons::json_parser_errc::expected_comma_or_right_bracket);
}

BOOST_AUTO_TEST_CASE(test_pass1)
{
    std::string in_file = "input/JSON_checker/pass1.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

BOOST_AUTO_TEST_CASE(test_pass2)
{
    std::string in_file = "input/JSON_checker/pass2.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

BOOST_AUTO_TEST_CASE(test_pass3)
{
    std::string in_file = "input/JSON_checker/pass3.json";

    int err = 0;

    try
    {
        json::parse_file(in_file);
    }
    catch (const parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

BOOST_AUTO_TEST_SUITE_END()


