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

// merge tests

TEST_CASE("test_json_merge")
{
json j = json::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");
json j2 = j;

const json source = json::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

const json expected = json::parse(R"(
{
    "a" : 1,
    "b" : 2,
    "c" : 3
}
)");

    SECTION("test 1")
    {
        j.merge(source);
        CHECK(j == expected);

        j2.merge(j2.object_range().begin()+1,source);
        CHECK(j2 == expected);
    }

    SECTION("test 2")
    {
        json empty_object;
        json original = j;

        j.merge(empty_object);

        CHECK(j == original);

        j2.merge(j2.object_range().begin()+1,empty_object);
        CHECK(j2 == original);
    }

    //std::cout << j << std::endl;
}

TEST_CASE("test_json_merge_move")
{
json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");
    json j2 = j;

json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json expected = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3],
    "c" : [4,5,6]
}
)");

    SECTION("test 1")
    {
        json source2 = source;

        j.merge(std::move(source));
        CHECK(j == expected);

        j2.merge(std::move(source2));
        CHECK(j2 == expected);
    }
}

// merge_or_update tests

TEST_CASE("test_json_merge_or_update")
{
json j = json::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");
json j2 = j;

const json source = json::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

const json expected = json::parse(R"(
{
    "a" : 2,
    "b" : 2,
    "c" : 3
}
)");

    SECTION("test 1")
    {
        j.merge_or_update(source);
        CHECK(j == expected);

        j2.merge_or_update(j2.object_range().begin()+1,source);
        CHECK(j2 == expected);
    }
}

TEST_CASE("test_json_merge_or_update_move")
{
json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");
    json j2 = j;

json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json expected = json::parse(R"(
{
    "a" : "2",
    "b" : [1,2,3],
    "c" : [4,5,6]
}
)");


    SECTION("test 1")
    {
        json source2 = source;

        j.merge_or_update(std::move(source));
        CHECK(j == expected);

        j2.merge_or_update(std::move(source2));
        CHECK(j2 == expected);
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


