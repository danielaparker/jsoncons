// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("json array deeply nested tests")
{
    SECTION("test 1")
    {
        jsoncons::json doc(jsoncons::json_array_arg);
        jsoncons::json* ref = &doc;
        for (std::size_t j = 0; j < 10000; ++j)
        {
            jsoncons::json val(jsoncons::json_array_arg, jsoncons::semantic_tag::none);
            ref->push_back(val);
            ref = &ref->at(0);
        }
    }
}

TEST_CASE("json_object deeply nested tests")
{
    SECTION("test 1")
    {
        jsoncons::json doc(jsoncons::json_object_arg);
        jsoncons::json* ref = &doc;
        for (std::size_t j = 0; j < 10000; ++j)
        {
            jsoncons::json val(jsoncons::json_object_arg, jsoncons::semantic_tag::none);
            ref->try_emplace("0",val);
            ref = &ref->at(0);
        }
    }
    SECTION("test 2")
    {
        jsoncons::ojson doc(jsoncons::json_object_arg);
        jsoncons::ojson* ref = &doc;
        for (std::size_t j = 0; j < 10000; ++j)
        {
            jsoncons::ojson val(jsoncons::json_object_arg, jsoncons::semantic_tag::none);
            ref->try_emplace("0",val);
            ref = &ref->at(0);
        }
    }
}

