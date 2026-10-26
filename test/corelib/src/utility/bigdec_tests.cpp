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
    SECTION("123.456 / 2 at scale 1")
    {
        jsoncons::bigdec a("123.456");
        jsoncons::bigdec b("2");
        jsoncons::bigdec expected("61.7");
        jsoncons::bigdec c;

        auto r = a.divide(b, c, 1);
        REQUIRE(r);
        CHECK(expected == c);
    }
    SECTION("1e-9223372036854775807 / 3 at scale 0")
    {
        jsoncons::bigdec a("1e-9223372036854775807");
        jsoncons::bigdec b("3");
        jsoncons::bigdec expected("0");
        jsoncons::bigdec c;

        auto r = a.divide(b, c, 0);
        REQUIRE(r);
        CHECK(expected == c);
    }
    SECTION("1e-1000000 / 3 at scale 0 rounding up")
    {
        jsoncons::bigdec a("1e-1000000");
        jsoncons::bigdec b("3");
        jsoncons::bigdec expected("1");
        jsoncons::bigdec c;

        auto r = a.divide(b, c, 0, jsoncons::rounding_mode::up);
        REQUIRE(r);
        CHECK(expected == c);
    }
    SECTION("a scale difference of 100000")
    {
        jsoncons::bigdec c;

        auto r = jsoncons::bigdec("1").divide(jsoncons::bigdec("1e-100000"), c, 0);
        REQUIRE(r);
        CHECK(jsoncons::bigdec::big_ten_to_the(100000) == c.unscaled());

        r = jsoncons::bigdec("1").divide(jsoncons::bigdec("3"), c, 100000);
        REQUIRE(r);
        CHECK(100000 == c.scale());
        CHECK(jsoncons::bigdec::big_ten_to_the(100000) / 3 == c.unscaled());
    }
    SECTION("a scale difference above 100000")
    {
        jsoncons::bigdec c;

        auto r = jsoncons::bigdec("1").divide(jsoncons::bigdec("1e-100001"), c, 0);
        CHECK(jsoncons::bignum_errc::result_out_of_range == r.ec);

        r = jsoncons::bigdec("1").divide(jsoncons::bigdec("3"), c, 100001);
        CHECK(jsoncons::bignum_errc::result_out_of_range == r.ec);

        r = jsoncons::bigdec("1").divide(jsoncons::bigdec("1e-9223372036854775807"), c, 0);
        CHECK(jsoncons::bignum_errc::result_out_of_range == r.ec);

        r = jsoncons::bigdec("0").divide(jsoncons::bigdec("1e-100001"), c, 0);
        REQUIRE(r);
        CHECK(jsoncons::bigdec("0") == c);

        r = jsoncons::bigdec("0").divide(jsoncons::bigdec("1e-9223372036854775807"), c, 0);
        REQUIRE(r);
        CHECK(jsoncons::bigdec("0") == c);
    }
}
