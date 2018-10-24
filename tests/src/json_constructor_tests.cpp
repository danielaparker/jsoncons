// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

TEST_CASE("json(string_view)")
{
    json::string_view_type sv("Hello world.");

    json j(sv);

    CHECK(j.as<json::string_view_type>() == sv);
    CHECK(j.as_string_view() == sv);
}

TEST_CASE("json(string, semantic_tag_type::date_time)")
{
    std::string s("2015-05-07 12:41:07-07:00");

    json j(s, semantic_tag_type::date_time);

    CHECK(j.semantic_tag() == semantic_tag_type::date_time);
    CHECK(j.as<std::string>() == s);
}

TEST_CASE("json(string, semantic_tag_type::epoch_time)")
{
    SECTION("positive integer")
    {
        int t = 10000;
        json j(t, semantic_tag_type::epoch_time);

        CHECK(j.semantic_tag() == semantic_tag_type::epoch_time);
        CHECK(j.as<int>() == t);
    }
    SECTION("negative integer")
    {
        int t = -10000;
        json j(t, semantic_tag_type::epoch_time);

        CHECK(j.semantic_tag() == semantic_tag_type::epoch_time);
        CHECK(j.as<int>() == t);
    }
    SECTION("floating point")
    {
        double t = 10000.1;
        json j(t, semantic_tag_type::epoch_time);

        CHECK(j.semantic_tag() == semantic_tag_type::epoch_time);
        CHECK(j.as<double>() == t);
    }

}

