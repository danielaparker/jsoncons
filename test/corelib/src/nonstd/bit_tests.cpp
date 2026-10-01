// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/nonstd/bit.hpp>
#include <catch/catch.hpp>

namespace nonstd = jsoncons::nonstd;

TEST_CASE("nonstd bit tests")
{
    SECTION("countl_zero")
    {
        CHECK(32 == nonstd::countl_zero(uint32_t(0)));
        CHECK(1 == nonstd::countl_zero(uint32_t(2147483647)));
        CHECK(0 == nonstd::countl_zero(uint32_t(4294967295)));
        CHECK(64 == nonstd::countl_zero(uint64_t(0)));
        CHECK(1 == nonstd::countl_zero(uint64_t(9223372036854775807)));
        CHECK(0 == nonstd::countl_zero(uint64_t(18446744073709551615)));
    }
    SECTION("countr_zero")
    {
        CHECK(32 == nonstd::countr_zero(uint32_t(0)));
        CHECK(0 == nonstd::countr_zero(uint32_t(2147483647)));
        CHECK(0 == nonstd::countr_zero(uint32_t(4294967295)));
        CHECK(64 == nonstd::countr_zero(uint64_t(0)));
        CHECK(0 == nonstd::countr_zero(uint64_t(9223372036854775807)));
        CHECK(0 == nonstd::countr_zero(uint64_t(18446744073709551615)));
    }
    SECTION("bit_width")
    {
        CHECK(0 == nonstd::bit_width(uint32_t(0)));
        CHECK(31 == nonstd::bit_width(uint32_t(2147483647)));
        CHECK(32 == nonstd::bit_width(uint32_t(4294967295)));
        CHECK(0 == nonstd::bit_width(uint64_t(0)));
        CHECK(63 == nonstd::bit_width(uint64_t(9223372036854775807)));
        CHECK(64 == nonstd::bit_width(uint64_t(18446744073709551615)));
    }
}


