
// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_array_extra_comma")
{

#if !defined(JSONCONS_NO_DEPRECATED)
    SECTION("using err_handler")
    {
        allow_trailing_commas err_handler;

        json expected = json::parse("[1,2,3]");

        auto options = json_options{}
        .err_handler(err_handler);
        json val = json::parse("[1,2,3,]", options);

        CHECK(expected == val);
    }
#endif
    SECTION("with option")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse("[1,2,3]");

        json val1 = json::parse("[1,2,3,]", options);

        CHECK(expected == val1);
    }
}

TEST_CASE("test_object_extra_comma")
{
    SECTION("with option")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse(R"(
    {
        "first" : 1,
        "second" : 2
    }
    )", options);

        json val = json::parse(R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )", options);

        CHECK(expected == val);
    }
}


