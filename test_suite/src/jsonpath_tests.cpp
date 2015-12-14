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
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

using jsoncons::jsonpath::json_query;
using jsoncons::pretty_print;
using jsoncons::json;

#if 0
BOOST_AUTO_TEST_CASE(test_jsonpath)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$.store.book");

    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1b)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$['store']['book']");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1b)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$['store']['book','bicycle']");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1c)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$['store']['book'][*]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpath2)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$.store..price");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpath_recursive_descent)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::string q = "$..book[2]";
    std::vector<json> result = json_query(val,q);

    //std::cout << q << std::endl;
    //for (size_t i = 0; i < result.size(); ++i)
    //{
    //    std::cout << pretty_print(result[i]) << std::endl;
    //}
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == val["store"]["book"][2]);
}

BOOST_AUTO_TEST_CASE(test_jsonpath_alternative_indices)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::string q = "$..book[0,1]";
    std::vector<json> result = json_query(val,q);

    //std::cout << q << std::endl;
    //for (size_t i = 0; i < result.size(); ++i)
    //{
    //    std::cout << pretty_print(result[i]) << std::endl;
    //}
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == val["store"]["book"][0]);
    BOOST_CHECK(result[1] == val["store"]["book"][1]);
}

BOOST_AUTO_TEST_CASE(test_jsonpath_start_end)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::string q = "$..book[:2]";
    std::vector<json> result = json_query(val,q);

    //std::cout << q << std::endl;
    //for (size_t i = 0; i < result.size(); ++i)
    //{
    //    std::cout << pretty_print(result[i]) << std::endl;
    //}

    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == val["store"]["book"][0]);
    BOOST_CHECK(result[1] == val["store"]["book"][1]);
}

BOOST_AUTO_TEST_CASE(test_jsonpath_start_end2)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::string q = "$..book[-1:]";
    std::vector<json> result = json_query(val,q);

    //std::cout << q << std::endl;
    //for (size_t i = 0; i < result.size(); ++i)
    //{
    //    std::cout << pretty_print(result[i]) << std::endl;
    //}
    BOOST_CHECK(result[0] == val["store"]["book"][3]);
}
#endif
#if 0
BOOST_AUTO_TEST_CASE(test_jsonpath_filter1)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$..book[?(@.price<10)]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
#endif
 
BOOST_AUTO_TEST_CASE(test_jsonpath_filter2)
{
    const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";

    json val = json::parse_string(text);

    std::vector<json> result = json_query(val,"$..book[?(10 > @.price)]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}


