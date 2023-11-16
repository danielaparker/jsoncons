// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <catch/catch.hpp>
#include <iostream>

using namespace jsoncons;

TEST_CASE("json_location tests")
{

    SECTION("test 1")
    {
        jsonpath::json_location loc;
        loc.append("foo").append(1);

        CHECK(loc.size() == 2);
        CHECK(loc[0].has_name());
        CHECK(loc[0].name() == "foo");
        CHECK(loc[1].has_index());
        CHECK(loc[1].index() == 1);
    }
}


TEST_CASE("json_location erase tests")
{

    std::string json_string = R"(
{ "store": {
    "book": [ 
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      },
      { "category": "fiction",
        "author": "Herman Melville",
        "title": "Moby Dick",
        "isbn": "0-553-21311-3",
        "price": 8.99
      }
    ]
  }
}
    )";

    json doc = json::parse(json_string);

    SECTION("store book 1")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(1);


        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][1]["author"].as<std::string>() == "Evelyn Waugh");

        std::size_t count = jsonpath::erase(doc, loc);

        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 2);
        CHECK(doc["store"]["book"][1]["author"].as<std::string>() == "Herman Melville");
    }

    SECTION("store book 2")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(2);


        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");

        std::size_t count = jsonpath::erase(doc, loc);

        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 2);
        CHECK(doc["store"]["book"][1]["author"].as<std::string>() == "Evelyn Waugh");
    }

    SECTION("store book 3")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(3);

        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");

        std::size_t count = jsonpath::erase(doc, loc);

        CHECK(count == 0);
        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");
    }

    SECTION("store")
    {
        jsonpath::json_location loc;
        loc.append("store");

        std::size_t count = jsonpath::erase(doc, loc);
        CHECK(count == 1);
        CHECK(doc.size() == 0);
    }

    SECTION("store book")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book");

        CHECK(doc["store"]["book"].size() == 3);
        std::size_t count = jsonpath::erase(doc, loc);
        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 0);
    }

    SECTION("store lost&found")
    {
        jsonpath::json_location loc;
        loc.append("store").append("lost&found");

        CHECK(doc["store"].size() == 1);
        std::size_t count = jsonpath::erase(doc, loc);
        CHECK(count == 0);
        CHECK(doc["store"].size() == 1);
    }

    SECTION("store book 2 price")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(2).append("price");


        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");
        CHECK(doc["store"]["book"][2].contains("price"));

        std::size_t count = jsonpath::erase(doc, loc);

        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");
        CHECK_FALSE(doc["store"]["book"][2].contains("price"));
    }
}

