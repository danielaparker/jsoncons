// Copyright 2019 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/detail/span.hpp>
#include <iostream>
#include <vector>

using jsoncons::detail::span;

TEST_CASE("span constructor tests")
{
    SECTION("span()")
    {
        span<const uint8_t> s;
    }
    SECTION("span(pointer,size_type)")
    {
        std::vector<uint8_t> v = {1,2,3,4};
        span<const uint8_t> s(v.data(), v.size());
        CHECK(s.size() == v.size());
        CHECK(s.data() == v.data());
    }
    SECTION("span(C& c)")
    {
        using C = std::vector<uint8_t>;
        C c = {1,2,3,4};

        span<const uint8_t> s(c);
        CHECK(s.size() == c.size());
        CHECK(s.data() == c.data());
    }
    SECTION("span(C c[])")
    {
        double c[] = {1,2,3,4};

        span<const double> s{ c };
        CHECK(s.size() == 4);
        CHECK(s.data() == c);
    }
    SECTION("span(std::array)")
    {
        std::array<double,4> c = {1,2,3,4};

        span<std::array<double,4>> s{ c };
        CHECK(s.size() == 4);
        CHECK(s.data() == c);
    }
}

