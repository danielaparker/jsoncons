
// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_array_extra_comma")
{
    allow_trailing_commas err_handler;

    json expected = json::parse("[1,2,3]");

    json_options options;
    options.err_handler(err_handler);
    json val = json::parse("[1,2,3,]", options);

    CHECK(val == expected);
}

TEST_CASE("test_object_extra_comma")
{
    allow_trailing_commas err_handler;

    json expected = json::parse(R"(
    {
        "first" : 1,
        "second" : 2
    }
    )", 
    err_handler);

    json val = json::parse(R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )", 
    err_handler);

    CHECK(val == expected);
}

TEST_CASE("test_name_without_quotes")
{
    //allow_trailing_commas err_handler;

    /*json val = json::parse(R"(
    {
        first : 1,
        second : 2
    }
    )", 
    err_handler);

    std::cout << val << std::endl;*/
}


