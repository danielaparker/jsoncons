// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/nonstd/u128.hpp>
#include <catch/catch.hpp>

using jsoncons::nonstd::u128;

TEST_CASE("u128 multiply")
{
    SECTION("carry out of the low word")
    {
        u128 x(0xFFFFFFFFFFFFFFFFULL);
        u128 y = x * x;
        CHECK(0xFFFFFFFFFFFFFFFEULL == y.hi);
        CHECK(1u == y.lo);
    }
    SECTION("high word")
    {
        u128 y = u128(0x100000000ULL, 0) * u128(3);
        CHECK(0x300000000ULL == y.hi);
        CHECK(0u == y.lo);
    }
}
