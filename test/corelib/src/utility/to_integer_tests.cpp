// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/utility/to_number.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("utility::to_integer tests")
{
    SECTION("")
    {
        std::string s = "-";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::invalid_argument);
    }
    SECTION("-")
    {
        std::string s = "-";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::invalid_argument);
    }
    SECTION("min int64_t")
    {
        std::string s = "-9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::min)());
    }
    SECTION("max int64_t")
    {
        std::string s = "9223372036854775807";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("max uint64_t")
    {
        std::string s = "18446744073709551615";
        uint64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<uint64_t>::max)());
    }
    SECTION("min int64_t - 1")
    {
        std::string s = "-9223372036854775809";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
    SECTION("max int64_t + 1")
    {
        std::string s = "9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
}

TEST_CASE("utility::decstr_to_integer tests")
{
    SECTION("")
    {
        std::string s = "-";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::invalid_argument);
    }
    SECTION("-")
    {
        std::string s = "-";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::invalid_argument);
    }
    SECTION("min int64_t")
    {
        std::string s = "-9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::min)());
    }
    SECTION("max int64_t")
    {
        std::string s = "9223372036854775807";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("min int64_t - 1")
    {
        std::string s = "-9223372036854775809";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
    SECTION("max int64_t + 1")
    {
        std::string s = "9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
}

TEST_CASE("utility::to_integer_unchecked tests")
{
    SECTION("max uint64_t")
    {
        std::string s = "18446744073709551615";
        uint64_t val;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<uint64_t>::max)());
    }
    SECTION("min int64_t")
    {
        std::string s = "-9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::min)());
    }
    SECTION("max int64_t")
    {
        std::string s = "9223372036854775807";
        int64_t val;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), val);
        REQUIRE(result);
        CHECK(val == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("min int64_t - 1")
    {
        std::string s = "-9223372036854775809";
        int64_t val;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
    SECTION("max int64_t + 1")
    {
        std::string s = "9223372036854775808";
        int64_t val;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), val);
        REQUIRE_FALSE(result);
        CHECK(result.ec == std::errc::result_out_of_range);
    }
}

