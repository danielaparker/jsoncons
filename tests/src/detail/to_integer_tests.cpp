// Copyright 2019 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("detail::to_integer tests")
{
    SECTION("")
    {
        std::string s = "-";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::invalid_number);
    }
    SECTION("-")
    {
        std::string s = "-";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::invalid_number);
    }
    SECTION("min int64_t")
    {
        std::string s = "-9223372036854775808";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE(result);
        CHECK(result.value() == (std::numeric_limits<int64_t>::min)());
    }
    SECTION("max int64_t")
    {
        std::string s = "9223372036854775807";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE(result);
        CHECK(result.value() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("min int64_t - 1")
    {
        std::string s = "-9223372036854775809";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::overflow);
    }
    SECTION("max int64_t + 1")
    {
        std::string s = "9223372036854775808";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::overflow);
    }
}

TEST_CASE("detail::to_integer_decimal tests")
{
    SECTION("")
    {
        std::string s = "-";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::invalid_number);
    }
    SECTION("-")
    {
        std::string s = "-";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::invalid_number);
    }
    SECTION("min int64_t")
    {
        std::string s = "-9223372036854775808";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE(result);
        CHECK(result.value() == (std::numeric_limits<int64_t>::min)());
    }
    SECTION("max int64_t")
    {
        std::string s = "9223372036854775807";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE(result);
        CHECK(result.value() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("min int64_t - 1")
    {
        std::string s = "-9223372036854775809";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::overflow);
    }
    SECTION("max int64_t + 1")
    {
        std::string s = "9223372036854775808";
        auto result = jsoncons::detail::to_integer_decimal<int64_t>(s.data(), s.length());
        REQUIRE_FALSE(result);
        CHECK(result.errc() == jsoncons::detail::to_integer_errc::overflow);
    }
}

