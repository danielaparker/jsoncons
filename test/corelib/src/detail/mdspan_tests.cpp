// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/detail/mdspan.hpp>
#include <mdspan>
#include <iostream>
#include <vector>
#include <catch/catch.hpp>

TEST_CASE("jsoncons::detail::mdspan constructor tests")
{
    SECTION("mdspan()")
    {
        std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        std::mdspan<int, std::dextents<std::size_t,2>> ms0;
        jsoncons::detail::mdspan<int, jsoncons::detail::dextents<std::size_t,2>> ms;
    }
}

