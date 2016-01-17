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

using namespace jsoncons;
using namespace jsoncons::jsonpath;

BOOST_AUTO_TEST_SUITE(jsonpath_error_test_suite)

struct jsonpath_fixture
{
    static const char* store_text()
    {
        static const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";
        return text;
    }
    static const char* store_text_empty_isbn()
    {
        static const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";
        return text;
    }
    static const char* book_text()
    {
        static const char* text = "{ \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95}";
        return text;
    }

    json book()
    {
        json root = json::parse(jsonpath_fixture::store_text());
        json book = root["store"]["book"];
        return book;
    }

    json bicycle()
    {
        json root = json::parse(jsonpath_fixture::store_text());
        json bicycle = root["store"]["bicycle"];
        return bicycle;
    }
};

BOOST_AUTO_TEST_CASE(test_root_error)
{

    json root = json::parse(jsonpath_fixture::store_text());

    try
    {
        json result = json_query(root,"..*");
    }
    catch (parse_exception& e)
    {
        std::cerr << "parse_exception: " << e.what() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
    }
    //std::cout << result << std::endl;
}

BOOST_AUTO_TEST_CASE(test_right_bracket_error)
{

    json root = json::parse(jsonpath_fixture::store_text());

    try
    {
        json result = json_query(root,"$['store']['book'[*]");
    }
    catch (parse_exception& e)
    {
        std::cerr << "parse_exception: " << e.what() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
    }
    //std::cout << result << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()




