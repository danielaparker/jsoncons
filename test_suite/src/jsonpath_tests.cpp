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

struct jsonpath_fixture
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

    json books()
    {
        json val = json::parse_string(jsonpath_fixture::store_text());
        json book = val["store"]["book"];
        return book;
    }
};

#if 0
BOOST_AUTO_TEST_CASE(test_jsonpath)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$.store.book");

    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1b)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$['store']['book']");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1b)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$['store']['book','bicycle']");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
BOOST_AUTO_TEST_CASE(test_jsonpath_1c)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$['store']['book'][*]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpath2)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$.store..price");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpath_recursive_descent)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    std::string q = "$..book[2]";
    json result = json_query(val,q);

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
    json val = json::parse_string(jsonpath_fixture::store_text());

    std::string q = "$..book[0,1]";
    json result = json_query(val,q);

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
    json val = json::parse_string(jsonpath_fixture::store_text());

    std::string q = "$..book[:2]";
    json result = json_query(val,q);

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
    json val = json::parse_string(jsonpath_fixture::store_text());

    std::string q = "$..book[-1:]";
    json result = json_query(val,q);

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
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book[?(@.price<10)]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
 
BOOST_AUTO_TEST_CASE(test_jsonpath_filter2)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book[?(10 > @.price)]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

 
BOOST_AUTO_TEST_CASE(test_jsonpath_filter2)
{
    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book[?(@.category == 'reference')]");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_jsonpath_filter3)
{
	jsonpath_fixture fixture;

    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book[?((@.price > 8) && (@.price < 12))]");

    json books = fixture.books();

    json expected = json::array();
    for (size_t i = 0; i < books.size(); ++i)
    {
        double price = books[i]["price"].as<double>();
        if (price > 8 && price < 12)
        {
            expected.add(books[i]);
        }
    }
	//std::cout << pretty_print(result) << std::endl;
    //std::cout << pretty_print(expected) << std::endl;

    BOOST_CHECK_EQUAL(expected,result);
}



BOOST_AUTO_TEST_CASE(test_jsonpath_book_isbn)
{
	jsonpath_fixture fixture;

    json val = json::parse_string(jsonpath_fixture::store_text());

    json books = fixture.books();
    for (size_t i = 0; i < books.size(); ++i)
    {
        bool has_isbn = books[i].has_member("isbn");
        if (has_isbn)
        {
            json result = json_query(books[i],"@.isbn");
            std::cout << pretty_print(result) << std::endl;
        }
    }


    //json result = json_query(val,"$..book[?(@.isbn)]");

    //json books = fixture.books();

    //json expected = json::array();
    //for (size_t i = 0; i < books.size(); ++i)
    //{
    //    double price = books[i]["price"].as<double>();
    //    if (price > 8 && price < 12)
    //    {
    //        expected.add(books[i]);
    //    }
    //}
	//std::cout << pretty_print(result) << std::endl;
    //std::cout << pretty_print(expected) << std::endl;

    //BOOST_CHECK_EQUAL(expected,result);
}

BOOST_AUTO_TEST_CASE(test_jsonpath_filter4)
{
	jsonpath_fixture fixture;

    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book[?(@.isbn)]");

    json books = fixture.books();

    //json expected = json::array();
    //for (size_t i = 0; i < books.size(); ++i)
    //{
    //    double price = books[i]["price"].as<double>();
    //    if (price > 8 && price < 12)
    //    {
    //        expected.add(books[i]);
    //    }
    //}
	std::cout << pretty_print(result) << std::endl;
    //std::cout << pretty_print(expected) << std::endl;

    //BOOST_CHECK_EQUAL(expected,result);
}
#endif

BOOST_AUTO_TEST_CASE(test_jsonpath_array_length)
{
	jsonpath_fixture fixture;

    json val = json::parse_string(jsonpath_fixture::store_text());

    json result = json_query(val,"$..book.length");

	std::cout << pretty_print(result) << std::endl;
}
 
#if 0
BOOST_AUTO_TEST_CASE(test_jsonpath_book_category)
{
    json val = json::parse_string(jsonpath_fixture::book_text());

    json result = json_query(val,"@.category");
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << pretty_print(result[i]) << std::endl;
    }
}
#endif


