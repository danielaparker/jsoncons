// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(parse_string_tests)

struct jsonpath_filter_fixture
{
};

class lenient_error_handler : public parse_error_handler
{
public:
    lenient_error_handler(std::error_code value)
        : value_(value)
    {
    }
private:
    std::error_code value_;

    bool do_error(std::error_code ec, const serializing_context&) JSONCONS_NOEXCEPT
    {
        return (ec != value_) ? true: false;
    }
};

BOOST_AUTO_TEST_CASE(test_parse_small_string1)
{
    std::string input = "\"String\"";
    std::istringstream is(input);

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is,decoder);
        reader.read_next();
    }
    catch (const std::exception&)
    {
    }
    BOOST_CHECK(decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_parse_small_string2)
{
    std::string input = "\"Str\\\"ing\"";
    std::istringstream is(input);

    json_decoder<json> decoder;
    try
    {
        json_reader reader(is, decoder);
        reader.read_next();
    }
    catch (const std::exception&)
    {
    }
    BOOST_CHECK(decoder.is_valid());
}

BOOST_AUTO_TEST_CASE(test_parse_small_string4)
{
    std::string input = "\"Str\\\"ing\"";

    for (size_t i = 2; i < input.length(); ++i)
    {
        std::istringstream is(input);
        json_decoder<json> decoder;
        try
        {
            json_reader reader(is, decoder);
            reader.buffer_length(i);
            reader.read_next();
        }
        catch (const std::exception&)
        {
        }
        BOOST_CHECK(decoder.is_valid());
        BOOST_CHECK_EQUAL(std::string("Str\"ing"),decoder.get_result().as<std::string>());
    }
}
BOOST_AUTO_TEST_CASE(test_parse_big_string1)
{
    std::string input = "\"Big Str\\\"ing\"";

    for (size_t i = 2; i < input.length(); ++i)
    {
        std::istringstream is(input);
        json_decoder<json> decoder;
        try
        {
            json_reader reader(is, decoder);
            reader.buffer_length(i);
            reader.read_next();
        }
        catch (const std::exception&)
        {
        }
        BOOST_CHECK(decoder.is_valid());
        BOOST_CHECK_EQUAL(std::string("Big Str\"ing"),decoder.get_result().as<std::string>());
    }
}

BOOST_AUTO_TEST_CASE(test_parse_big_string2)
{
    std::string input = "\"Big\t Str\\\"ing\"";

    //for (size_t i = 2; i < input.length(); ++i)
    //{
        std::istringstream is(input);
        json_decoder<json> decoder;
        lenient_error_handler err_handler(json_parse_errc::illegal_character_in_string);
        try
        {
            json_reader reader(is, decoder, err_handler);
            //reader.buffer_length(i);
            reader.read_next();
        }
        catch (const std::exception&)
        {
        }
        BOOST_CHECK(decoder.is_valid());
        BOOST_CHECK_EQUAL(std::string("Big\t Str\"ing"),decoder.get_result().as<std::string>());
    //}
}

BOOST_AUTO_TEST_SUITE_END()


