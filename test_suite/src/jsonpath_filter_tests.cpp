// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/json_query.hpp"

using jsoncons::jsonpath::json_query;
using jsoncons::jsonpath::jsonpath_filter_parser;
using jsoncons::pretty_print;
using jsoncons::json;

struct jsonpath_filter_fixture
{
    static const char* store_text()
    {
        static const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";
        return text;
    }
    static const char* book_text()
    {
        static const char* text = "{ \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95}";
        return text;
    }

    json book()
    {
        json root = json::parse_string(jsonpath_filter_fixture::store_text());
        json book = root["store"]["book"];
        return book;
    }

    json bicycle()
    {
        json root = json::parse_string(jsonpath_filter_fixture::store_text());
        json bicycle = root["store"]["bicycle"];
        return bicycle;
    }
};

BOOST_AUTO_TEST_CASE(test_jsonpath_filter)
{
    jsonpath_filter_parser<char,std::allocator<void>> parser;
	json parent = json::array();
	parent.add(1);
	parent.add(2);

	std::string expr1 = "(1 + 1)";
    parser.parse(expr1.c_str(),0,expr1.length(),1,1);
    auto result1 = parser.eval(parent);
    BOOST_CHECK_EQUAL(json(2),result1);

	std::string expr2 = "(1 - 1)";
	parser.parse(expr2.c_str(), 0, expr2.length(), 1, 1);
	auto result2 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(0), result2);

	std::string expr3 = "(@.length - 1)";
	parser.parse(expr3.c_str(), 0, expr3.length(), 1, 1);
	auto result3 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(1), result3);

}

BOOST_AUTO_TEST_CASE(test_jsonpath_index_expression)
{
    jsonpath_filter_fixture fixture;

    json root = json::parse_string(jsonpath_filter_fixture::store_text());

    json result = json_query(root,"$..book[(@.length-1)]");

    BOOST_CHECK_EQUAL(1,result.size());
    BOOST_CHECK_EQUAL(root["store"]["book"][3],result[0]);

    //    std::cout << pretty_print(result) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_jsonpath_filter_negative_numbers)
{
    jsonpath_filter_parser<char,std::allocator<void>> parser;
    json parent = json::array();
    parent.add(1);
    parent.add(2);

    std::string expr1 = "(-1 + 1)";
    parser.parse(expr1.c_str(),0,expr1.length(),1,1);
    auto result1 = parser.eval(parent);
    BOOST_CHECK_EQUAL(json(0),result1);

	std::string expr2 = "(1 + -1)";
	parser.parse(expr2.c_str(), 0, expr2.length(), 1, 1);
	auto result2 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(0), result2);

	std::string expr3 = "(-1 - -1)";
	parser.parse(expr3.c_str(), 0, expr3.length(), 1, 1);
	auto result3 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(0), result3);

	std::string expr4 = "(-1 - -3)";
	parser.parse(expr4.c_str(), 0, expr4.length(), 1, 1);
	auto result4 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(2), result4);

    std::string expr5 = "((-2 < -1) && (-3 > -4))";
    parser.parse(expr5.c_str(), 0, expr5.length(), 1, 1);
    auto result5 = parser.eval(parent);
    BOOST_CHECK_EQUAL(json(true), result5);

	std::string expr6 = "((-2 < -1) || (-4 > -3))";
	parser.parse(expr6.c_str(), 0, expr6.length(), 1, 1);
	auto result6 = parser.eval(parent);
	BOOST_CHECK_EQUAL(json(true), result6);
}

BOOST_AUTO_TEST_CASE(test_jsonpath_filter_uni)
{
    jsonpath_filter_parser<char,std::allocator<void>> parser;
	json parent = json::array();
	parent.add(1);
	parent.add(2);

	std::string expr1 = "(0)";
    parser.parse(expr1.c_str(),0,expr1.length(),1,1);
    auto result1 = parser.eval(parent);
    BOOST_CHECK_EQUAL(json(0),result1);
}




