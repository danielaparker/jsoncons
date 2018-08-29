// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

// https://jsonpath.herokuapp.com/

const json store = json::parse(R"(
{
    "store": {
        "book": [
            {
                "category": "reference",
                "author": "Nigel Rees",
                "title": "Sayings of the Century",
                "price": 8.95
            },
            {
                "category": "fiction",
                "author": "Evelyn Waugh",
                "title": "Sword of Honour",
                "price": 12.99
            },
            {
                "category": "fiction",
                "author": "Herman Melville",
                "title": "Moby Dick",
                "isbn": "0-553-21311-3",
                "price": 8.99
            },
            {
                "category": "fiction",
                "author": "J. R. R. Tolkien",
                "title": "The Lord of the Rings",
                "isbn": "0-395-19395-8",
                "price": 22.99
            }
        ],
        "bicycle": {
            "color": "red",
            "price": 19.95
        }
    },
    "expensive": 10
}
)");

TEST_CASE("test_normalized_paths")
{

const json expected = json::parse(R"(
[
   "$['store']['book'][0]['category']"
]
)");

    std::string path = "$.store.book[0].category";

    json result = json_query(store,"$.store.book[0].category",result_type::path);
    CHECK(expected == result);
}

TEST_CASE("test_string_index")
{

const json expected = json::parse(R"(
[
   "$['store']['book'][0]['category'][0]",
   "$['store']['book'][0]['category'][2]"
]
)");

    std::string path = "$.store.book.0.category[0,2]";

    json result = json_query(store,path,result_type::path);
    CHECK(expected == result);

    //json result2 = json_query(store,path,result_type::value);
    //std::cout << pretty_print(result2) << std::endl;
}

TEST_CASE("test_array_length")
{

const json expected = json::parse(R"(
[
    "$['store']['book']['length']"
]
)");

    std::string path = "$.store.book.length";
    json result = json_query(store,path,result_type::path);
    CHECK(expected == result);

    std::string path2 = "$.store.book['length']";
    json result2 = json_query(store, path, result_type::path);
    CHECK(expected == result2);
}

TEST_CASE("test_price_filter")
{

const json expected = json::parse(R"(
[
    "$['store']['book'][0]['title']",
    "$['store']['book'][2]['title']"
]
)");

    std::string path = "$.store.book[?(@.price < 10)].title";
    json result = json_query(store,path,result_type::path);
    CHECK(expected == result);
}

TEST_CASE("test_length_expression")
{

const json expected = json::parse(R"(
[
     "$['store']['book'][3]['title']"
]
)");

    std::string path = "$.store.book[(@.length-1)].title";
    json result = json_query(store,path,result_type::path);
    CHECK(expected == result);
}




