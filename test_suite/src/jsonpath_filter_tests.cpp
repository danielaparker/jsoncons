// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

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
    std::string expr = "(1+1)";

    parser.parse(expr.c_str(),0,expr.length(),1,1);
    auto filter = parser.get_filter();
    json parent;
    auto result = filter->evaluate(parent);
    std::cout << result << std::endl;

}





