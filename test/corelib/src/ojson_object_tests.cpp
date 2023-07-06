// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>

using namespace jsoncons;

TEST_CASE("ojson insert(first,last) test")
{
    SECTION("copy map into ojson")
    {
        std::map<std::string,double> m1 = {{"f",4},{"e",5},{"d",6}};
        std::map<std::string,double> m2 = {{"c",1},{"b",2},{"a",3}};

        ojson j;
        j.insert(m1.begin(),m1.end());
        j.insert(m2.begin(),m2.end());

        //std::cout << j << "\n";

        REQUIRE(j.size() == 6);
        auto it = j.object_range().begin();
        CHECK(it++->key() == "d");
        CHECK(it++->key() == "e");
        CHECK(it++->key() == "f");
        CHECK(it++->key() == "a");
        CHECK(it++->key() == "b");
        CHECK(it++->key() == "c");
    }
}

TEST_CASE("ojson parse_duplicate_names")
{
    ojson oj1 = ojson::parse(R"({"first":1,"second":2,"third":3})");
    CHECK(3 == oj1.size());
    CHECK(1 == oj1["first"].as<int>());
    CHECK(2 == oj1["second"].as<int>());
    CHECK(3 == oj1["third"].as<int>());

    ojson oj2 = ojson::parse(R"({"first":1,"second":2,"first":3})");
    CHECK(2 == oj2.size());
    CHECK(1 == oj2["first"].as<int>());
    CHECK(2 == oj2["second"].as<int>());
}

TEST_CASE("ojson object erase with iterator")
{
    SECTION("ojson erase with iterator")
    {
        ojson j(jsoncons::json_object_arg);

        j.try_emplace("a", 1);
        j.try_emplace("b", 2);
        j.try_emplace("c", 3);

        auto it = j.object_range().begin();
        while (it != j.object_range().end())
        {
            if (it->key() == "a" || it->key() == "c")
            {
                it = j.erase(it);
            }
            else
            {
                it++;
            }
        }

        CHECK(j.size() == 1);
        CHECK(j.at("b") == 2);
        CHECK(j["b"] == 2);
    }

    SECTION("ojson erase with iterator 2")
    {
        ojson j(jsoncons::json_object_arg);

        j.try_emplace("a", 1);
        j.try_emplace("b", 2);
        j.try_emplace("c", 3);

        auto it = j.object_range().begin();
        while (it != j.object_range().end())
        {
            if (it->key() == "a")
            {
                it = j.erase(it, it+2);
            }
            else
            {
                it++;
            }
        }

        CHECK(j.size() == 1);
        CHECK(j.at("c") == 3);
        CHECK(j["c"] == 3);
    }

    SECTION("ojson erase with iterator 3")
    {
        ojson j(jsoncons::json_object_arg);

        j.try_emplace("c", 1);
        j.try_emplace("b", 2);
        j.try_emplace("a", 3);

        auto it = j.object_range().begin();
        while (it != j.object_range().end())
        {
            if (it->key() == "c")
            {
                it = j.erase(it, it+2);
            }
            else
            {
                it++;
            }
        }

        CHECK(j.size() == 1);
        CHECK(j.at("a") == 3);
        CHECK(j["a"] == 3);
    }
}


TEST_CASE("test_ojson_merge")
{
ojson j = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3,
    "d" : 4,
    "b" : 5,
    "e" : 6
}
)");

    SECTION("merge j with source")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 1,
            "b" : 2,
            "c" : 3,
            "d" : 4,
            "e" : 6
        }
        )");
        j.merge(source);
        CHECK(j == expected);
    }

    SECTION("merge j")
    {
        const ojson expected = ojson::parse(R"(
{"a":1,"c":3,"d":4,"b":2,"e":6}
        )");
        j.merge(j.object_range().begin()+1,source);
        CHECK(j == expected);
    }

    //std::cout << j << std::endl;
}

TEST_CASE("test_ojson_merge_move")
{
ojson j = ojson::parse(R"(
{
    "a" : "1",
    "d" : [1,2,3]
}
)");

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");
    SECTION("merge source into j")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "1",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        j.merge(std::move(source));
        CHECK(j == expected);
    }
    SECTION("merge source into j at begin")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "1",
            "c" : [4,5,6],
            "d" : [1,2,3]
        }
        )");

        j.merge(j.object_range().begin(),std::move(source));
        CHECK(j == expected);
    }


    //std::cout << "(1)\n" << j << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}

TEST_CASE("test_ojson_merge_or_update")
{
ojson j = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

    SECTION("merge_or_update source into j")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 2,
            "b" : 2,
            "c" : 3
        }
        )");
        j.merge_or_update(source);
        CHECK(j == expected);
    }

    SECTION("merge_or_update source into j at pos 1")
    {
        const ojson expected = ojson::parse(R"(
        {
            "a" : 2,
            "c" : 3,
            "b" : 2
        }
        )");
        j.merge_or_update(j.object_range().begin()+1,source);
        CHECK(j == expected);
    }

    //std::cout << j << std::endl;
}

TEST_CASE("test_ojson_merge_or_update_move")
{
ojson j = ojson::parse(R"(
{
    "a" : "1",
    "d" : [1,2,3]
}
)");

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

    SECTION("merge or update j from source")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "2",
            "d" : [1,2,3],
            "c" : [4,5,6]
        }
        )");

        j.merge_or_update(std::move(source));
        //CHECK(j == expected);
    }
    SECTION("merge or update j from source at pos")
    {
        ojson expected = ojson::parse(R"(
        {
            "a" : "2",
            "c" : [4,5,6],
            "d" : [1,2,3]
        }
        )");

        j.merge_or_update(j.object_range().begin(),std::move(source));
        CHECK(j.size() == 3);
        CHECK(j == expected);
    }

    //std::cout << "(1)\n" << j << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}



