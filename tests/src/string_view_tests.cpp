// Copyright 2019 Daniel Parker
// Distributed under Boost license

#if defined(__GNUC__) && __GNUC__ == 8

#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cwchar>
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

    auto s = j.as<std::string_view>();
    CHECK(bool(s == "2"));
}

#endif

