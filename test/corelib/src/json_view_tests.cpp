// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_view.hpp>
#include <jsoncons/basic_json.hpp>

#include <catch/catch.hpp>

TEST_CASE("json_view constructor tests")
{
    SECTION("ref")
    {
        jsoncons::json j{100};
        jsoncons::json_view<jsoncons::json> jv{j};
        CHECK(jv.is<int>());
        jsoncons::json_view<jsoncons::json> jv2{jv};
        CHECK(jv2.is<int>());
    }
    SECTION("const ref")
    {
        jsoncons::json j{100};
        jsoncons::json_view<const jsoncons::json> jv{j};
        CHECK(jv.is<int>());
        jsoncons::json_view<const jsoncons::json> jv2{jv};
        CHECK(jv2.is<int>());
    }
}


