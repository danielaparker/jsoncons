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

BOOST_AUTO_TEST_CASE(test_object2)
{
json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

    std::cout << source << std::endl;
}

BOOST_AUTO_TEST_CASE(test_object_with_three_members)
{
    std::string input = "{\"A\":\"Jane\", \"B\":\"Roe\",\"C\":10}";
    json val = json::parse(input);

    BOOST_CHECK_EQUAL(true,val.is_object());
    BOOST_CHECK_EQUAL(3,val.size());
}

BOOST_AUTO_TEST_CASE(test_double)
{
    json val = json::parse("42.229999999999997");
    BOOST_CHECK_EQUAL(17,val.precision());
}

BOOST_AUTO_TEST_CASE(test_array_of_integer)
{
    std::string s = "[1,2,3]";
    json j1 = json::parse(s);
    BOOST_CHECK_EQUAL(true,j1.is_array());
    BOOST_CHECK_EQUAL(3,j1.size());

    std::istringstream is(s);
    json j2 = json::parse(is);
    BOOST_CHECK_EQUAL(true,j2.is_array());
    BOOST_CHECK_EQUAL(3,j2.size());
}

BOOST_AUTO_TEST_CASE(test_skip_bom)
{
    std::string s = "\xEF\xBB\xBF[1,2,3]";
    json j1 = json::parse(s);
    BOOST_CHECK_EQUAL(true,j1.is_array());
    BOOST_CHECK_EQUAL(3,j1.size());

    std::istringstream is(s);
    json j2 = json::parse(is);
    BOOST_CHECK_EQUAL(true,j2.is_array());
    BOOST_CHECK_EQUAL(3,j2.size());
}

BOOST_AUTO_TEST_CASE(test_empty_object)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s("{}");

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
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

    parser.set_source(s.data(),s.length());
    parser.parse_some();
    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_CASE(test_array_string)
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);

    parser.reset();

    static std::string s1("[\"\"");

    parser.set_source(s1.data(),s1.length());
    parser.parse_some();
    BOOST_CHECK(!parser.done());
    static std::string s2("]");
    parser.set_source(s2.data(), s2.length());
    parser.parse_some();

    BOOST_CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();
}

BOOST_AUTO_TEST_SUITE_END()




