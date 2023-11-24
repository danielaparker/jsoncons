// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_location.hpp>
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

TEST_CASE("json_location parse tests")
{
    SECTION("test 1")
    {
        jsonpath::json_location loc;
        loc.append("foo").append(1);

        jsonpath::json_location loc2 = jsonpath::json_location::parse("$['foo'][1]");
        CHECK(loc2 == loc);
    }
}

TEST_CASE("json_location remove tests")
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

        std::size_t count = jsonpath::remove(doc, loc);

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

        std::size_t count = jsonpath::remove(doc, loc);

        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 2);
        CHECK(doc["store"]["book"][1]["author"].as<std::string>() == "Evelyn Waugh");
    }

    SECTION("store book 3")
    {
        json orig = doc;

        jsonpath::json_location loc;
        loc.append("store").append("book").append(3);

        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");

        std::size_t count = jsonpath::remove(doc, loc);

        CHECK(count == 0);
        CHECK(doc == orig);
    }

    SECTION("store")
    {
        jsonpath::json_location loc;
        loc.append("store");

        std::size_t count = jsonpath::remove(doc, loc);
        CHECK(count == 1);
        CHECK(doc.size() == 0);
    }

    SECTION("store book")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book");

        CHECK(doc["store"]["book"].size() == 3);
        std::size_t count = jsonpath::remove(doc, loc);
        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 0);
    }

    SECTION("store lost&found")
    {
        json orig = doc;

        jsonpath::json_location loc;
        loc.append("store").append("lost&found");

        CHECK(doc["store"].size() == 1);
        std::size_t count = jsonpath::remove(doc, loc);
        CHECK(count == 0);
        CHECK(doc == orig);
    }

    SECTION("store book 2 price")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(2).append("price");

        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");
        CHECK(doc["store"]["book"][2].contains("price"));

        std::size_t count = jsonpath::remove(doc, loc);

        CHECK(count == 1);
        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2]["author"].as<std::string>() == "Herman Melville");
        CHECK_FALSE(doc["store"]["book"][2].contains("price"));
    }

    SECTION("store 0")
    {
        json orig = doc;

        jsonpath::json_location loc;
        loc.append("store").append(0);

        CHECK(doc["store"]["book"].size() == 3);
        CHECK(doc["store"]["book"][2].contains("price"));

        std::size_t count = jsonpath::remove(doc, loc);

        CHECK(count == 0);
        CHECK(doc == orig);
    }
}

TEST_CASE("json_location select tests")
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

        json* p_json = jsonpath::get(doc, loc);

        CHECK_FALSE(p_json == nullptr);
        CHECK(*p_json == doc.at("store").at("book").at(1));
    }

    SECTION("store book 2")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(2);

        json* p_json = jsonpath::get(doc, loc);

        CHECK_FALSE(p_json == nullptr);
        CHECK(*p_json == doc.at("store").at("book").at(2));
    }

    SECTION("store book 3")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(3);

        json* p_json = jsonpath::get(doc, loc);

        CHECK(p_json == nullptr);
    }

    SECTION("store")
    {
        jsonpath::json_location loc;
        loc.append("store");

        json* p_json = jsonpath::get(doc, loc);
        CHECK_FALSE(p_json == nullptr);
        CHECK(*p_json == doc.at("store"));
    }

    SECTION("store book")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book");

        json* p_json = jsonpath::get(doc, loc);
        CHECK_FALSE(p_json == nullptr);
        CHECK(*p_json == doc.at("store").at("book"));
    }

    SECTION("store lost&found")
    {
        jsonpath::json_location loc;
        loc.append("store").append("lost&found");

        json* p_json = jsonpath::get(doc, loc);
        CHECK(p_json == nullptr);
    }

    SECTION("store book 2 price")
    {
        jsonpath::json_location loc;
        loc.append("store").append("book").append(2).append("price");

        json* p_json = jsonpath::get(doc, loc);

        CHECK_FALSE(p_json == nullptr);
        CHECK(*p_json == doc.at("store").at("book").at(2).at("price"));
    }

    SECTION("store 0")
    {
        jsonpath::json_location loc;
        loc.append("store").append(0);

        json* p_json = jsonpath::get(doc, loc);

        CHECK(p_json == nullptr);
    }
}

TEST_CASE("test json_location from path_node")
{
    SECTION("test 1")
    {
        jsonpath::path_node a1{};
        jsonpath::path_node a2(&a1,"foo");
        jsonpath::path_node a3(&a2,"bar");
        jsonpath::path_node a4(&a3,7);

        jsonpath::json_location location;
        location.append("foo").append("bar").append(7);

        std::string jsonpath_string = "$['foo']['bar'][7]";

        CHECK((jsonpath::json_location{ a4 } == location));
        CHECK((jsonpath::to_string(location) == jsonpath_string));
    }
}

