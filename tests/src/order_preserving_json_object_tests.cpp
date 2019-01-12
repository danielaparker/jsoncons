// Copyright 2019 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

TEST_CASE("ojson parse_duplicate_names")
{
    ojson oj1 = ojson::parse(R"({"first":1,"second":2,"third":3})");
    CHECK(3 == oj1.size());
    CHECK(1 == oj1["first"].as<int>());
    CHECK(2 == oj1["second"].as<int>());
    CHECK(3 == oj1["third"].as<int>());

    ojson oj2 = ojson::parse(R"({"first":1,"second":2,"first":3})");
    CHECK(2 == oj2.size());
    CHECK(3 == oj2["first"].as<int>());
    CHECK(2 == oj2["second"].as<int>());
}

TEST_CASE("test_ojson_merge")
{
ojson j = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

ojson j2 = j;

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");
const ojson expected = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2,
    "c" : 3
}
)");

    j.merge(source);
    CHECK(j.size() == 3);
    CHECK(j == expected);

    j2.merge(j2.object_range().begin()+1,source);
    CHECK(j2.size() == 3);
    CHECK(expected == j2);

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
ojson j2 = j;

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

ojson source2 = source;

ojson expected = ojson::parse(R"(
{
    "d" : [1,2,3],
    "a" : "1",
    "c" : [4,5,6]
}
)");

    j.merge(std::move(source));
    CHECK(j.size() == 3);
    CHECK(j == expected);

    j2.merge(j2.object_range().begin(),std::move(source2));
    CHECK(j2.size() == 3);
    CHECK(expected == j2);

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

ojson j2 = j;

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");
const ojson expected = ojson::parse(R"(
{
    "a" : 2,
    "b" : 2,
    "c" : 3
}
)");

    j.merge_or_update(source);
    CHECK(j.size() == 3);
    CHECK(j == expected);

    j2.merge_or_update(j2.object_range().begin()+1,source);
    CHECK(j2.size() == 3);
    CHECK(expected == j2);

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
ojson j2 = j;

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

ojson source2 = source;

ojson expected = ojson::parse(R"(
{
    "d" : [1,2,3],
    "a" : "2",
    "c" : [4,5,6]
}
)");

    j.merge_or_update(std::move(source));
    CHECK(j.size() == 3);
    CHECK(j == expected);

    j2.merge_or_update(j2.object_range().begin(),std::move(source2));
    CHECK(j2.size() == 3);
    CHECK(expected == j2);

    //std::cout << "(1)\n" << j << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}

