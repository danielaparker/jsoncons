// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/detail/optional.hpp>
#include <jsoncons/json.hpp>

using jsoncons::detail::optional;
using jsoncons::json;

TEST_CASE("optional constructor tests")
{
    SECTION("optional<T>()")
    {
        optional<int> x;
        CHECK_FALSE(x.has_value());
        CHECK_FALSE(x);
    }
    SECTION("optional<T>(json)")
    {
        std::string input = R"(
        [
            {
                "enrollmentNo" : 100,
                "firstName" : "Tom",
                "lastName" : "Cochrane",
                "mark" : 55              
            },
            {
                "enrollmentNo" : 101,
                "firstName" : "Catherine",
                "lastName" : "Smith",
                "mark" : 95
            },
            {
                "enrollmentNo" : 102,
                "firstName" : "William",
                "lastName" : "Skeleton",
                "mark" : 60              
            }
        ]
        )";
        json j = json::parse(input);
        optional<json> x(j);
        CHECK(x.has_value());
        bool b(x);
        CHECK(b);

        json* p = x.operator->();
        REQUIRE(p);
        REQUIRE(p->size() == 3);
        json& ref = x.value();
        REQUIRE(ref.size() == 3);

        const auto& cref = *x;
        REQUIRE(cref.size() == 3);

        x = j[1];
        REQUIRE(x.has_value());
        const auto& cref2 = *x;
        REQUIRE(cref2.is_object());
        REQUIRE(cref2.size() == 4);
        CHECK(cref2["firstName"].as<std::string>() == std::string("Catherine"));
    }
}

