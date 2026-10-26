// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/utility/bigdec.hpp>
#include <iostream>
#include <limits>
#include <catch/catch.hpp>

namespace {

void test_bigdec(jsoncons::string_view sv, jsoncons::string_view expected)
{
    jsoncons::bigdec value;
    auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
    CHECK(result);
    CHECK(sv.data() + sv.length() == result.ptr);

    std::string buf;
    append_to_buffer(value, buf);
    CHECK(expected == buf);
}

void test_bigdec(jsoncons::wstring_view sv, jsoncons::wstring_view expected)
{
    jsoncons::bigdec value;
    auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
    CHECK(result);
    CHECK(sv.data() + sv.length() == result.ptr);

    std::wstring buf;
    append_to_buffer(value, buf);
    CHECK(expected == buf);
}

} // namespace

TEST_CASE("basic_bigdec success tests")
{
    SECTION("char")
    {
        test_bigdec("0", "0");
        test_bigdec("123456", "123456");
        test_bigdec("123456.123456", "123456.123456");
        test_bigdec("-123456", "-123456");
        test_bigdec("-123456.123456", "-123456.123456");
        test_bigdec("123456e5", "1.23456e10");
        test_bigdec("123456e+5", "1.23456e10");
        test_bigdec("123456E5", "1.23456e10");
        test_bigdec("123456E+5", "1.23456e10");
        test_bigdec("123456e-5", "1.23456");
        test_bigdec("123456E-5", "1.23456");
        test_bigdec("123456e-6", "0.123456");
        test_bigdec("123456e-7", "0.0123456");
        test_bigdec("123456.123456e7", "1.23456123456e12");
        test_bigdec("123456.123456e-7", "0.0123456123456");
    }
    SECTION("wchar_t")
    {
        test_bigdec(L"0", L"0");
        test_bigdec(L"123456", L"123456");
        test_bigdec(L"123456.123456", L"123456.123456");
        test_bigdec(L"-123456", L"-123456");
        test_bigdec(L"-123456.123456", L"-123456.123456");
        test_bigdec(L"123456e5", L"1.23456e10");
        test_bigdec(L"123456e+5", L"1.23456e10");
        test_bigdec(L"123456E5", L"1.23456e10");
        test_bigdec(L"123456E+5", L"1.23456e10");
        test_bigdec(L"123456e-5", L"1.23456");
        test_bigdec(L"123456E-5", L"1.23456");
        test_bigdec(L"123456e-6", L"0.123456");
        test_bigdec(L"123456e-7", L"0.0123456");
        test_bigdec(L"123456.123456e7", L"1.23456123456e12");
        test_bigdec(L"123456.123456e-7", L"0.0123456123456");
    }
}

TEST_CASE("basic_bigdec extreme scale tests")
{
    std::string buf;

    append_to_buffer(jsoncons::bigdec{jsoncons::bigint{1234567890123456789}, (std::numeric_limits<int64_t>::min)()}, buf);
    CHECK(buf == "1.234567890123456789e9223372036854775826");

    buf.clear();
    append_to_buffer(jsoncons::bigdec{jsoncons::bigint{1234567890123456789}, -(std::numeric_limits<int64_t>::max)()}, buf);
    CHECK(buf == "1.234567890123456789e9223372036854775825");

    buf.clear();
    append_to_buffer(jsoncons::bigdec{jsoncons::bigint{5}, (std::numeric_limits<int64_t>::max)()}, buf);
    CHECK(buf == "5e-9223372036854775807");
}

TEST_CASE("basic_bigdec terminal tests")
{
    SECTION("10[1]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "10";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        append_to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("1[.]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "1.";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        append_to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("12.12[e]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "12.12e";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        append_to_buffer(value, buf);
        CHECK(expected == buf);
    }
}

TEST_CASE("basic_bigdec multiply tests")
{
    SECTION("123456.123456 * 123456789.123456789")
    {
        jsoncons::bigdec a("123456.123456");
        jsoncons::bigdec b("123456789.123456789");
        jsoncons::bigdec expected("15241496599506.839368265342784");
        jsoncons::bigdec c;
        auto r = jsoncons::multiply(a, b, c);
        REQUIRE(r);
        CHECK(expected == c);
    }
}

TEST_CASE("basic_bigdec divide tests")
{
    SECTION("4.0 / 2.0")
    {
        jsoncons::bigdec a("4.0");
        jsoncons::bigdec b("2.0");
        jsoncons::bigdec expected("2");
        jsoncons::bigdec c;

        auto r = a.divide(b, c);
        REQUIRE(r);
        CHECK((expected == c));
    }
    SECTION("12345678901234567890.0987654321 / 12345678901234567890.12345678901234567890")
    {
        jsoncons::bigdec a("12345678901234567890.0987654321");
        jsoncons::bigdec b("12345678901234567890.12345678901234567890");
        jsoncons::bigdec expected("0.9999999999999999999980000000721000");

        jsoncons::bigdec c;

        auto r = a.divide(b, c);
        REQUIRE(r);
        CHECK(expected == c);
    }
    SECTION("12345678901234567890.12345678901234567890 / 12345678901234567890.0987654321")
    {
        jsoncons::bigdec a("12345678901234567890.0987654321");
        jsoncons::bigdec b("12345678901234567890.12345678901234567890");
        jsoncons::bigdec expected("1.0000000000000000000019999999279000");

        jsoncons::bigdec c;

        auto r = b.divide(a, c);
        REQUIRE(r);
        CHECK(expected == c);
    }
}

TEST_CASE("basic_bigdec compare tests")
{
    SECTION("scales more than int64_t apart")
    {
        jsoncons::bigdec small("1e-9223372036854775807");
        jsoncons::bigdec large("1e9223372036854775807");

        CHECK(small.compare(large) < 0);
        CHECK(large.compare(small) > 0);
        CHECK(small.compare(jsoncons::bigdec("1")) < 0);
        CHECK(large.compare(jsoncons::bigdec("1")) > 0);
    }
    SECTION("scales near the cut-off")
    {
        CHECK(0 == jsoncons::bigdec("1").compare(jsoncons::bigdec("1.000")));
        CHECK(jsoncons::bigdec("1").compare(jsoncons::bigdec("1.001")) < 0);
        CHECK(jsoncons::bigdec("1.001").compare(jsoncons::bigdec("1")) > 0);
    }
    SECTION("a 400-bit value near the cut-off")
    {
        // 2^399 is about 1.29e120, and the cut-off for 400 bits is a distance of 134
        CHECK(jsoncons::bigdec(jsoncons::bigint(1) << 399, 120).compare(jsoncons::bigdec("1")) > 0);
        CHECK(jsoncons::bigdec(jsoncons::bigint(1) << 399, 121).compare(jsoncons::bigdec("1")) < 0);
        CHECK(jsoncons::bigdec(jsoncons::bigint(1) << 399, 133).compare(jsoncons::bigdec("1")) < 0);
        CHECK(jsoncons::bigdec(jsoncons::bigint(1) << 399, 134).compare(jsoncons::bigdec("1")) < 0);
    }
    SECTION("zero")
    {
        CHECK(jsoncons::bigdec("0").compare_abs(jsoncons::bigdec("1e-5")) < 0);
        CHECK(jsoncons::bigdec("1e-5").compare_abs(jsoncons::bigdec("0")) > 0);
        CHECK(0 == jsoncons::bigdec("0").compare_abs(jsoncons::bigdec("0.00")));
    }
}
