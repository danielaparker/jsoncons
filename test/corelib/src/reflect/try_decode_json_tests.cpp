// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/decode_json.hpp>

#include <map>
#include <vector>

#include <catch/catch.hpp>

TEST_CASE("reflect decode json")
{
    SECTION("string source")
    {
        std::string s = R"([false,"foo"])";
        auto j = jsoncons::reflect::decode_json<jsoncons::json>(s);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0] == false);
        CHECK(j[1] == "foo");
    }
}
