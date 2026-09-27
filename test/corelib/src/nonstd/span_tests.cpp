// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/nonstd/span.hpp>
#include <iostream>
#include <vector>
#include <catch/catch.hpp>

TEST_CASE("jsoncons::nonstd::span constructor tests")
{
    SECTION("jsoncons::nonstd::span()")
    {
        jsoncons::nonstd::span<const uint8_t> s;
        CHECK(s.empty());
    }
    SECTION("jsoncons::nonstd::span(pointer,size_type)")
    {
        std::vector<uint8_t> v = {1,2,3,4};
        jsoncons::nonstd::span<const uint8_t> s(v.data(), v.size());
        CHECK(s.size() == v.size());
        CHECK(s.data() == v.data());
    }
    SECTION("jsoncons::nonstd::span(C& c)")
    {
        using C = std::vector<uint8_t>;
        C c = {{1,2,3,4}};

        jsoncons::nonstd::span<const uint8_t> s(c);
        CHECK(s.size() == c.size());
        CHECK(s.data() == c.data());
    }
    SECTION("jsoncons::nonstd::span(C c[])")
    {
        double c[] = {1,2,3,4};

        jsoncons::nonstd::span<const double> s{ c };
        CHECK(4 == s.size());
        CHECK(s.data() == c);
    }
    SECTION("jsoncons::nonstd::span(std::array)")
    {
        std::array<double,4> c = {{1,2,3,4}};

        jsoncons::nonstd::span<double> s(c);
        CHECK(4 == s.size());
        CHECK(s.data() == c.data());
    }
}

