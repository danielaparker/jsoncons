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

        const json root_value = json::parse(input);
        const json original = root_value;

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [&](const std::string& /*location*/, const json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                ++count;
            }
        };

        expr.evaluate(root_value, op);

        CHECK(count == 1);
        CHECK(root_value == original);
    }

    SECTION("evaluate with std::error_code")
    {
        int count = 0;

        const json root_value = json::parse(input);
        const json original = root_value;

        std::error_code ec;
        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]", ec);
        CHECK_FALSE(ec);

        auto op = [&](const std::string& /*location*/, const json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                ++count;
            }
        };

        expr.evaluate(root_value, op);

        CHECK(count == 1);
        CHECK(root_value == original);
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

        const json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [&](const jsonpath::path_node& /*path*/, const json& value)
        {
            if (value.at("category") == "memoir" && !value.contains("price"))
            {
                ++count;
                //std::cout << jsonpath::to_string(path) << ": " << value << "\n";
            }
        };

        expr.select(root_value, op);

        CHECK(count == 1);
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
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::json_location> paths = expr.select_paths(root_value);

        REQUIRE(paths.size() == 4);
        CHECK(jsonpath::to_string(paths[0]) == "$['books'][0]");
        CHECK(jsonpath::to_string(paths[1]) == "$['books'][1]");
        CHECK(jsonpath::to_string(paths[2]) == "$['books'][2]");
        CHECK(jsonpath::to_string(paths[3]) == "$['books'][3]");
    }

    SECTION("Return locations of selected values")
    {
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]['category','title']");

        std::vector<jsonpath::json_location> paths = expr.select_paths(root_value,jsonpath::result_options::nodups | jsonpath::result_options::sort_descending);

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

    SECTION("Return locations, nodups, sort_descending")
    {
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]['category','category','title','title']");

        std::vector<jsonpath::json_location> paths = expr.select_paths(root_value,jsonpath::result_options::nodups | jsonpath::result_options::sort_descending);

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

    SECTION("Return locations, sort_descending")
    {
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]['category','category','title','title']");

        std::vector<jsonpath::json_location> paths = expr.select_paths(root_value, jsonpath::result_options::sort_descending);

        REQUIRE(paths.size() == 16);
        CHECK(jsonpath::to_string(paths[0]) == "$['books'][3]['title']");
        CHECK(jsonpath::to_string(paths[1]) == "$['books'][3]['title']");
        CHECK(jsonpath::to_string(paths[2]) == "$['books'][3]['category']");
        CHECK(jsonpath::to_string(paths[3]) == "$['books'][3]['category']");
        CHECK(jsonpath::to_string(paths[4]) == "$['books'][2]['title']");
        CHECK(jsonpath::to_string(paths[5]) == "$['books'][2]['title']");
        CHECK(jsonpath::to_string(paths[6]) == "$['books'][2]['category']");
        CHECK(jsonpath::to_string(paths[7]) == "$['books'][2]['category']");
        CHECK(jsonpath::to_string(paths[8]) == "$['books'][1]['title']");
        CHECK(jsonpath::to_string(paths[9]) == "$['books'][1]['title']");
        CHECK(jsonpath::to_string(paths[10]) == "$['books'][1]['category']");
        CHECK(jsonpath::to_string(paths[11]) == "$['books'][1]['category']");
        CHECK(jsonpath::to_string(paths[12]) == "$['books'][0]['title']");
        CHECK(jsonpath::to_string(paths[13]) == "$['books'][0]['title']");
        CHECK(jsonpath::to_string(paths[14]) == "$['books'][0]['category']");
        CHECK(jsonpath::to_string(paths[15]) == "$['books'][0]['category']");

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
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        auto op = [](const jsonpath::path_node& /*location*/, json& book)
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                book.try_emplace("price", 140.0);
            }
        };

        expr.update(root_value, op);

        CHECK(root_value["books"][3].contains("price"));
        CHECK(root_value["books"][3].at("price") == 140);
    }

    SECTION("Return locations of selected values")
    {
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::json_location> paths = expr.select_paths(root_value);

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
        json root_value = json::parse(input);

        auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

        std::vector<jsonpath::path_node> path_nodes;
        auto callback2 = [&](const jsonpath::path_node& base_node, json&)
        {
            path_nodes.push_back(base_node);
        };

        expr.update(root_value, callback2);

        REQUIRE(path_nodes.size() == 4);
        CHECK(path_nodes[0].index() == 3);
        CHECK(path_nodes[1].index() == 2);
        CHECK(path_nodes[2].index() == 1);
        CHECK(path_nodes[3].index() == 0);
    }
}

TEST_CASE("jsonpath_expression remove")
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
        json doc = json::parse(input);

        json expected = doc;
        expected["books"].erase(expected["books"].array_range().begin()+3);
        expected["books"].erase(expected["books"].array_range().begin(),expected["books"].array_range().begin()+2);

        std::size_t n = jsoncons::jsonpath::remove(doc, "$.books[1,1,3,3,0,0]");

        CHECK(n == 3);
        REQUIRE(doc.at("books").size() == 1);
        CHECK(doc == expected);
    }
}
