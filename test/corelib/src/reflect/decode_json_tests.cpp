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
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.has_value());
        REQUIRE(result.value().is_array());
        REQUIRE(result.value().size() == 2);
        CHECK(result.value()[0] == false);
        CHECK(result.value()[1] == "foo");
    }
}
