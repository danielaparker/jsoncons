// Copyright 2013 Daniel Parker
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

BOOST_AUTO_TEST_SUITE(json_parse_error_tests)

void test_parse_error(const std::string& text, std::error_code ec)
{
    try
    {
        json::parse(text);
        BOOST_FAIL(text);
    }
    catch (const parse_error& e)
    {
        if (e.code() != ec)
        {
            std::cout << text << std::endl;
            std::cout << e.code().value() << " " << e.what() << std::endl; 
        }
        BOOST_CHECK_EQUAL(ec, e.code());
    }
}

void test_parse_ec(const std::string& text, std::error_code expected)
{
    std::error_code ec;

    std::istringstream is(text);
    json_decoder<json> decoder;
    json_reader reader(is,decoder);

    reader.read(ec);
    //std::cerr << text << std::endl;
    //std::cerr << ec.message() 
    //          << " at line " << reader.line_number() 
    //          << " and column " << reader.column_number() << std::endl;

    BOOST_CHECK(ec);
    BOOST_CHECK_EQUAL(expected,ec);
}

BOOST_AUTO_TEST_CASE(test_missing_separator)
{
    std::string jtext = R"({"field1"{}})";    

    test_parse_error(jtext, jsoncons::json_parse_errc::expected_colon);
    test_parse_ec(jtext, jsoncons::json_parse_errc::expected_colon);
}

BOOST_AUTO_TEST_CASE(test_invalid_value)
{
    std::string jtext = R"({"field1":ru})";    

    test_parse_error(jtext,jsoncons::json_parse_errc::expected_value);
    test_parse_ec(jtext, jsoncons::json_parse_errc::expected_value);
}


BOOST_AUTO_TEST_CASE(test_unexpected_end_of_file)
{
    std::string jtext = R"({"field1":{})";    

    test_parse_error(jtext, jsoncons::json_parse_errc::unexpected_eof);
    test_parse_ec(jtext, jsoncons::json_parse_errc::unexpected_eof);
}

BOOST_AUTO_TEST_CASE(test_value_not_found)
{
    std::string jtext = R"({"name":})";    

    test_parse_error(jtext, jsoncons::json_parse_errc::expected_value);
    test_parse_ec(jtext, jsoncons::json_parse_errc::expected_value);
}

BOOST_AUTO_TEST_CASE(test_escaped_characters)
{
    std::string input("[\"\\n\\b\\f\\r\\t\"]");
    std::string expected("\n\b\f\r\t");

    json o = json::parse(input);
    BOOST_CHECK(expected == o[0].as<std::string>());
}


BOOST_AUTO_TEST_CASE(test_expected_colon)
{
    test_parse_error("{\"name\" 10}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" true}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" false}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" null}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" \"value\"}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" {}}", jsoncons::json_parse_errc::expected_colon);
    test_parse_error("{\"name\" []}", jsoncons::json_parse_errc::expected_colon);
}

BOOST_AUTO_TEST_CASE(test_expected_name)
{
    test_parse_error("{10}", jsoncons::json_parse_errc::expected_name);
    test_parse_error("{true}", jsoncons::json_parse_errc::expected_name);
    test_parse_error("{false}", jsoncons::json_parse_errc::expected_name);
    test_parse_error("{null}", jsoncons::json_parse_errc::expected_name);
    test_parse_error("{{}}", jsoncons::json_parse_errc::expected_name);
    test_parse_error("{[]}", jsoncons::json_parse_errc::expected_name);
}

BOOST_AUTO_TEST_CASE(test_expected_value)
{
    test_parse_error("[tru]", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("[fa]", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("[n]", jsoncons::json_parse_errc::invalid_value);
}

BOOST_AUTO_TEST_CASE(test_parse_primitive_pass)
{
    json val;
    BOOST_CHECK_NO_THROW((val=json::parse("null")));
    BOOST_CHECK_EQUAL(val,json::null());
    BOOST_CHECK_NO_THROW((val=json::parse("false")));
    BOOST_CHECK_EQUAL(val,json(false));
    BOOST_CHECK_NO_THROW((val=json::parse("true")));
    BOOST_CHECK_EQUAL(val,json(true));
    BOOST_CHECK_NO_THROW((val=json::parse("10")));
    BOOST_CHECK_EQUAL(val,json(10));
    BOOST_CHECK_NO_THROW((val=json::parse("1.999")));
    BOOST_CHECK_EQUAL(val,json(1.999));
    BOOST_CHECK_NO_THROW((val=json::parse("\"string\"")));
    BOOST_CHECK_EQUAL(val,json("string"));
}

BOOST_AUTO_TEST_CASE(test_parse_empty_structures)
{
    json val;
    BOOST_CHECK_NO_THROW((val=json::parse("{}")));
    BOOST_CHECK_NO_THROW((val=json::parse("[]")));
    BOOST_CHECK_NO_THROW((val=json::parse("{\"object\":{},\"array\":[]}")));
    BOOST_CHECK_NO_THROW((val=json::parse("[[],{}]")));
}

BOOST_AUTO_TEST_CASE(test_parse_primitive_fail)
{
    test_parse_error("null {}", jsoncons::json_parse_errc::extra_character);
    test_parse_error("n ", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("nu ", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("nul ", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("false {}", jsoncons::json_parse_errc::extra_character);
    test_parse_error("fals ", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("true []", jsoncons::json_parse_errc::extra_character);
    test_parse_error("tru ", jsoncons::json_parse_errc::invalid_value);
    test_parse_error("10 {}", jsoncons::json_parse_errc::extra_character);
    test_parse_error("1a ", jsoncons::json_parse_errc::invalid_number);
    test_parse_error("1.999 []", jsoncons::json_parse_errc::extra_character);
    test_parse_error("1e0-1", jsoncons::json_parse_errc::invalid_number);
    test_parse_error("\"string\"{}", jsoncons::json_parse_errc::extra_character);
    test_parse_error("\"string\"[]", jsoncons::json_parse_errc::extra_character);
}

BOOST_AUTO_TEST_CASE(test_multiple)
{
    std::string in="{\"a\":1,\"b\":2,\"c\":3}{\"a\":4,\"b\":5,\"c\":6}";
    //std::cout << in << std::endl;

    std::istringstream is(in);

    jsoncons::json_decoder<json> decoder;
    json_reader reader(is,decoder);

    if (!reader.eof())
    {
        reader.read_next();
        BOOST_CHECK(!reader.eof());
        json val = decoder.get_result();
        BOOST_CHECK_EQUAL(1,val["a"].as<int>());
    }
    if (!reader.eof())
    {
        reader.read_next();
        BOOST_CHECK(!reader.eof());
        json val = decoder.get_result();
        BOOST_CHECK_EQUAL(4,val["a"].as<int>());
    }
}

BOOST_AUTO_TEST_CASE(test_uinteger_overflow)
{
    uint64_t m = (std::numeric_limits<uint64_t>::max)();
    std::string s1 = std::to_string(m);
    std::string s2 = s1;
    s2.push_back('0');
    
    json j1 =  json::parse(s1);
    BOOST_CHECK(j1.is_uinteger());
    BOOST_CHECK(m == j1.as_uinteger());

    json j2 =  json::parse(s2);
    BOOST_CHECK(!j2.is_uinteger());
    BOOST_CHECK(j2.is_bignum());
    BOOST_CHECK_EQUAL(s2,j2.as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_negative_integer_overflow)
{
    int64_t m = (std::numeric_limits<int64_t>::min)();
    std::string s1 = std::to_string(m);
    std::string s2 = s1;
    s2.push_back('0');
    
    json j1 =  json::parse(s1);
    BOOST_CHECK(m == j1.as_integer());

    json j2 =  json::parse(s2);
    BOOST_CHECK(!j2.is_integer());
    BOOST_CHECK(j2.is_bignum());
    BOOST_CHECK_EQUAL(s2,j2.as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_positive_integer_overflow)
{
    int64_t m = (std::numeric_limits<int64_t>::max)();
    std::string s1 = std::to_string(m);
    std::string s2 = s1;
    s2.push_back('0');

    json j1 =  json::parse(s1);
    BOOST_CHECK(m == j1.as_integer());

    json j2 =  json::parse(s2);
    BOOST_CHECK(!j2.is_integer());
    BOOST_CHECK(j2.is_bignum());
    BOOST_CHECK_EQUAL(s2,j2.as<std::string>());
}

BOOST_AUTO_TEST_SUITE_END()




