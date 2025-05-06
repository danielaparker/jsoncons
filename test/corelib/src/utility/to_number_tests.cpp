// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/utility/to_number.hpp>
#include <clocale>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("jsoncons::utility::to_integer tests")
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
    SECTION("max uint64_t")
    {
        std::string s = std::to_string((std::numeric_limits<uint64_t>::max)());
        uint64_t value{ 0 };
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), value);
        REQUIRE(result);
        CHECK(value == (std::numeric_limits<uint64_t>::max)());
        s.push_back('0');
        auto result2 = jsoncons::utility::decstr_to_integer(s.data(), s.length(), value);
        REQUIRE_FALSE(result2);
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
        std::string s = std::to_string((std::numeric_limits<uint64_t>::max)());
        uint64_t value{ 0 };
        auto result = jsoncons::utility::decstr_to_integer(s.data(), s.length(), value);
        REQUIRE(result);
        CHECK(value == (std::numeric_limits<uint64_t>::max)());
        s.push_back('0');
        auto result2 = jsoncons::utility::decstr_to_integer(s.data(), s.length(), value);
        REQUIRE_FALSE(result2);
    }
    SECTION("min int64_t")
    {
        std::string s = std::to_string((std::numeric_limits<int64_t>::lowest)());
        int64_t value;
        auto result = jsoncons::utility::to_integer_unchecked(s.data(), s.length(), value);
        REQUIRE(result);
        CHECK(value == (std::numeric_limits<int64_t>::lowest)());
        s.push_back('0');
        auto result2 = jsoncons::utility::decstr_to_integer(s.data(), s.length(), value);
        REQUIRE_FALSE(result2);
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

TEST_CASE("jsoncons::utility::to_double tests")
{
    SECTION("minus overflow")
    {
        std::string s = "-50011731000E95978";
        double val;
        auto result = jsoncons::utility::to_double(s.data(), s.length(), val);
        REQUIRE(result.ec == std::errc::result_out_of_range);
        auto result2 = jsoncons::utility::to_double(s.c_str(), s.length(), val);
        REQUIRE(result2.ec == std::errc::result_out_of_range);
    }
    SECTION("overflow")
    {
        std::wstring s = L"50011731000E95978";
        double val;
        auto result = jsoncons::utility::to_double(s.data(), s.length(), val);
        REQUIRE(result.ec == std::errc::result_out_of_range);
        auto result2 = jsoncons::utility::to_double(s.c_str(), s.length(), val);
        REQUIRE(result2.ec == std::errc::result_out_of_range);
    }
    SECTION("locale")
    {
        std::string prev_loc = std::setlocale(LC_ALL, nullptr);
        if (std::setlocale(LC_NUMERIC, "de_DE.UTF-8"))
        {
            std::string s = "1234.5678";
            std::wstring ws = L"1234.5678";
            
            double val1{0};
            auto result1 = jsoncons::utility::to_double(&s[0], s.length(), val1);
            CHECK(result1);
            CHECK(1234.5678 == val1);

            double val2{ 0 };
            auto result2 = jsoncons::utility::to_double(&ws[0], ws.length(), val2);
            CHECK(result2);
            CHECK(1234.5678 == val2);
            
            double val3{0};
            auto result3 = jsoncons::utility::to_double(s.c_str(), s.length(), val3);
            CHECK(result3);
            CHECK(1234.5678 == val3);

            double val4{ 0 };
            auto result4 = jsoncons::utility::to_double(ws.c_str(), ws.length(), val4);
            CHECK(result4);
            CHECK(1234.5678 == val4);
        }
        std::setlocale(LC_ALL, prev_loc.c_str());
    }
}
