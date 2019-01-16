// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/detail/grisu3.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <cstdio>

using namespace jsoncons;

template <class Result>
bool safe_dtoa(double val, Result& result)
{
    if (val == 0)
    {
        result.push_back('0');
        result.push_back('.');
        result.push_back('0');
        return true;
    }

    jsoncons::detail::string_to_double to_double_;

    char buffer[100];
    int precision = std::numeric_limits<double>::digits10;
    int length = snprintf(buffer, sizeof(buffer), "%1.*g", precision, val);
    if (length < 0)
    {
        return false;
    }
    int precision2 = std::numeric_limits<double>::max_digits10;
    if (to_double_(buffer,sizeof(buffer)) != val)
    {
        length = snprintf(buffer, sizeof(buffer), "%1.*g", precision2, val);
        if (length < 0)
        {
            return false;
        }
    }
    bool needs_dot = true;
    for (size_t i = 0; i < length; ++i)
    {
        switch (buffer[i])
        {
            case '.':
            case 'e':
            case 'E':
                needs_dot = false;
                break;
        }
        result.push_back(buffer[i]);
    }
    if (needs_dot)
    {
        result.push_back('.');
        result.push_back('0');
    }
    return true;
}

template <class Result>
bool dtoa(double v, Result& result)
{
    if (v == 0)
    {
        result.push_back('0');
        result.push_back('.');
        result.push_back('0');
        return true;
    }

    int length = 0;
    int k;

    char buffer[100];

    double u = std::signbit(v) ? -v : v;
    if (jsoncons::detail::grisu3(u, buffer, &length, &k))
    {
        if (std::signbit(v))
        {
            result.push_back('-');
        }
        jsoncons::detail::prettify_string(buffer, length, k, -6, 21, result);
        return true;
    }
    else
    {
        return safe_dtoa(v, result);
    }
}

static void check_safe_dtoa(double x, const std::vector<std::string>& expected)
{
    std::string s;
    bool result = safe_dtoa(x, s);
    if (!result)
    {
        std::cout << "safe_dtoa failed " << s << "\n";
    }
    REQUIRE(result);

    bool accept = false;
    for (size_t i = 0; !accept && i < expected.size(); ++i)
    {
        accept = s == expected[i];
    }
    if (!accept)
    {
        std::cout << "safe_dtoa does not match expected " << x << " " << s << "\n";
    }

    CHECK(accept);
}

static void check_dtoa(double x, const std::vector<std::string>& expected)
{
    std::string s;
    bool result = dtoa(x, s);
    if (!result)
    {
        std::cout << "dtoa failed " << s << "\n";
    }
    REQUIRE(result);

    bool accept = false;
    for (size_t i = 0; !accept && i < expected.size(); ++i)
    {
        accept = s == expected[i];
    }
    if (!accept)
    {
        std::cout << "dtoa does not match expected " << x << " " << s << "\n";
    }
    CHECK(accept);

    check_safe_dtoa(x,expected);
}

TEST_CASE("test grisu3")
{
    check_dtoa(1.0e100, {"1e+100","1e100"});
    check_dtoa(1.0e-100, {"1e-100"});
    check_dtoa(0.123456789e-100, {"1.23456789e-101"});
    check_dtoa(0.123456789e100, {"1.23456789e+99","1.23456789e99"});

    check_dtoa(1234563, {"1.234563e+6,1.234563e6","1234563.0"});

    check_dtoa(0.0000001234563, {"1.234563e-07","1.234563e-7"});

    check_dtoa(-1.0e+100, {"-1e+100","-1e100"});

    check_dtoa(-1.0e-100, {"-1e-100"});

    check_dtoa(0, {"0.0"});
    check_dtoa(-0, {"0.0"});
    check_dtoa(1, {"1.0"});
    check_dtoa(0.1, {"0.1"});

    check_dtoa(1.1, {"1.1"});

    check_dtoa(-1, {"-1.0"});
    check_dtoa(10, {"10.0"});
    check_dtoa(-10, {"-10.0"});
    check_dtoa(-11, {"-11.0"});
}

