// Copyright 2019 Daniel Parker
// Distributed under Boost license

#if defined(JSON_HAS_CPP_17)

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <string_view>

using namespace jsoncons;

TEST_CASE("string_view tests")
{
    json j = json::parse(R"(
    {
        "a" : "2",
        "c" : [4,5,6]
    }
    )");

    auto s = j["a"].as<std::string_view>();
    CHECK(bool(s == "2"));
}

#endif

