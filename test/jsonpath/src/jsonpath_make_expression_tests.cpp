// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <unordered_set> // std::unordered_set
#include <fstream>

using namespace jsoncons;

TEST_CASE("jsonpath make_expression::evaluate tests")
{
    std::string input = R"(
    {
        "books":
        [
            {
                "category": "fiction",
                "title" : "A Wild Sheep Chase",
                "author" : "Haruki Murakami",
                "price" : 22.72
            },
            {
                "category": "fiction",
                "title" : "The Night Watch",
                "author" : "Sergei Lukyanenko",
                "price" : 23.58
            },
            {
                "category": "fiction",
                "title" : "The Comedians",
                "author" : "Graham Greene",
                "price" : 21.99
            },
            {
                "category": "memoir",
                "title" : "The Night Watch",
                "author" : "Phillips, David Atlee"
            }
        ]
    }
    )";

    SECTION("test 1")
    {
        int count = 0;

        const json doc = json::parse(input);
        const json original = doc;

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [&](const std::string& /*location*/, const json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                ++count;
            }
        };

        expr.evaluate(doc, op);

        CHECK(count == 1);
        CHECK(doc == original);
    }
}

TEST_CASE("jsonpath_expression::select tests")
{
    std::string input = R"(
    {
        "books":
        [
            {
                "category": "fiction",
                "title" : "A Wild Sheep Chase",
                "author" : "Haruki Murakami",
                "price" : 22.72
            },
            {
                "category": "fiction",
                "title" : "The Night Watch",
                "author" : "Sergei Lukyanenko",
                "price" : 23.58
            },
            {
                "category": "fiction",
                "title" : "The Comedians",
                "author" : "Graham Greene",
                "price" : 21.99
            },
            {
                "category": "memoir",
                "title" : "The Night Watch",
                "author" : "Phillips, David Atlee"
            }
        ]
    }
    )";

    SECTION("test 1")
    {
        int count = 0;

        const json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [&](const jsonpath::path_node& /*location*/, const json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                ++count;
            }
        };

        expr.select(doc, op);

        CHECK(count == 1);
        CHECK_FALSE(doc["books"][3].contains("price"));
    }
}

TEST_CASE("jsonpath_expression::select_path tests")
{
    std::string input = R"(
    {
        "books":
        [
            {
                "category": "fiction",
                "title" : "A Wild Sheep Chase",
                "author" : "Haruki Murakami",
                "price" : 22.72
            },
            {
                "category": "fiction",
                "title" : "The Night Watch",
                "author" : "Sergei Lukyanenko",
                "price" : 23.58
            },
            {
                "category": "fiction",
                "title" : "The Comedians",
                "author" : "Graham Greene",
                "price" : 21.99
            },
            {
                "category": "memoir",
                "title" : "The Night Watch",
                "author" : "Phillips, David Atlee"
            }
        ]
    }
    )";

    SECTION("Return locations of selected values")
    {
        json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::json_location> paths = expr.select_paths(doc);

        REQUIRE(paths.size() == 4);
        CHECK(jsonpath::to_string(paths[0]) == "$['books'][0]");
        CHECK(jsonpath::to_string(paths[1]) == "$['books'][1]");
        CHECK(jsonpath::to_string(paths[2]) == "$['books'][2]");
        CHECK(jsonpath::to_string(paths[3]) == "$['books'][3]");
    }

    SECTION("Return locations of selected values")
    {
        json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]['category','title']");

        std::vector<jsonpath::json_location> paths = expr.select_paths(doc,jsonpath::result_options::nodups| jsonpath::result_options::sort_descending);

        REQUIRE(paths.size() == 8);
        CHECK(jsonpath::to_string(paths[0]) == "$['books'][3]['title']");
        CHECK(jsonpath::to_string(paths[1]) == "$['books'][3]['category']");
        CHECK(jsonpath::to_string(paths[2]) == "$['books'][2]['title']");
        CHECK(jsonpath::to_string(paths[3]) == "$['books'][2]['category']");
        CHECK(jsonpath::to_string(paths[4]) == "$['books'][1]['title']");
        CHECK(jsonpath::to_string(paths[5]) == "$['books'][1]['category']");
        CHECK(jsonpath::to_string(paths[6]) == "$['books'][0]['title']");
        CHECK(jsonpath::to_string(paths[7]) == "$['books'][0]['category']");

        //for (const auto& path : paths)
        //{
        //    std::cout << jsonpath::to_string(path) << "\n";
        //}
    }
}

TEST_CASE("jsonpath_expression::update tests")
{
    std::string input = R"(
    {
        "books":
        [
            {
                "category": "fiction",
                "title" : "A Wild Sheep Chase",
                "author" : "Haruki Murakami",
                "price" : 22.72
            },
            {
                "category": "fiction",
                "title" : "The Night Watch",
                "author" : "Sergei Lukyanenko",
                "price" : 23.58
            },
            {
                "category": "fiction",
                "title" : "The Comedians",
                "author" : "Graham Greene",
                "price" : 21.99
            },
            {
                "category": "memoir",
                "title" : "The Night Watch",
                "author" : "Phillips, David Atlee"
            }
        ]
    }
    )";

    SECTION("Update in place")
    {
        json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [](const jsonpath::path_node& /*location*/, json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                book.try_emplace("price", 140.0);
            }
        };

        expr.update(doc, op);

        CHECK(doc["books"][3].contains("price"));
        CHECK(doc["books"][3].at("price") == 140);
    }

    SECTION("Return locations of selected values")
    {
        json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::json_location> paths = expr.select_paths(doc);

        REQUIRE(paths.size() == 4);
        CHECK(jsonpath::to_string(paths[0]) == "$['books'][0]");
        CHECK(jsonpath::to_string(paths[1]) == "$['books'][1]");
        CHECK(jsonpath::to_string(paths[2]) == "$['books'][2]");
        CHECK(jsonpath::to_string(paths[3]) == "$['books'][3]");
        //for (const auto& path : paths)
        //{
        //    std::cout << jsonpath::to_string(path) << "\n";
        //}
    }

    SECTION("update default sort order")
    {
        json doc = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::path_node> path_nodes;
        auto callback2 = [&](const jsonpath::path_node& base_node, json&)
        {
            path_nodes.push_back(base_node);
        };

        expr.update(doc, callback2);

        REQUIRE(path_nodes.size() == 4);
        CHECK(path_nodes[0].index() == 3);
        CHECK(path_nodes[1].index() == 2);
        CHECK(path_nodes[2].index() == 1);
        CHECK(path_nodes[3].index() == 0);
    }
}


