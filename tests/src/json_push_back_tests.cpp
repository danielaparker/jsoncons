// Copyright 2013 Daniel Parker
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

TEST_CASE("json array deeply nested tests")
{
    SECTION("test 1")
    {
        json doc(json_array_arg);
        json* ref = &doc;
        for (size_t j = 0; j < 10000; ++j)
        {
            json val(json_array_arg, semantic_tag::none);
            ref->push_back(val);
            ref = &ref->at(0);
        }
    }
}

TEST_CASE("json_object deeply nested tests")
{
    SECTION("test 1")
    {
        json doc(json_object_arg);
        json* ref = &doc;
        for (size_t j = 0; j < 10000; ++j)
        {
            json val(json_object_arg, semantic_tag::none);
            ref->try_emplace("0",val);
            ref = &ref->at(0);
        }
    }
    SECTION("test 2")
    {
        ojson doc(json_object_arg);
        ojson* ref = &doc;
        for (size_t j = 0; j < 10000; ++j)
        {
            ojson val(json_object_arg, semantic_tag::none);
            ref->try_emplace("0",val);
            ref = &ref->at(0);
        }
    }
}

