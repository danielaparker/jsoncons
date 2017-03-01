// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_decoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_parser_tests)

BOOST_AUTO_TEST_CASE(test_object)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("{}");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_array)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("[]");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_string)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("\"\"");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_integer)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("10");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(!parser.done());

    parser.end_parse();
    BOOST_CHECK(parser.done());

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_integer_space)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("10 ");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_double_space)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("10.0 ");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_false)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("false");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_true)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("true");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_null)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("null");

    parser.set_buffer(s.data(),s.length());
    parser.parse();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_SUITE_END()




