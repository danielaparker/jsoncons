// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/typed_array.hpp>
#include <iostream>
#include <catch/catch.hpp>

void func(const std::vector<int>& data, 
    const std::vector<std::size_t>& extents, 
    const std::vector<std::size_t>& strides, 
    std::vector<std::size_t> indices,
    std::size_t index)
{
    if (index+1 == extents.size())
    {
        std::size_t offset = jsoncons::get_offset(strides, indices);
        std::cout << "[";
        for (std::size_t i = 0; i < extents[index]; ++i)
        {
            if (i > 0)
            {
                std::cout << ",";
            }
            std::cout << data[offset];
            offset += strides[index];
        }
        std::cout << "]";
    }
    else
    {
        std::cout << "[";
        for (std::size_t i = 0; i < extents[index]; ++i)
        {
            if (i > 0)
            {
                std::cout << ",";
            }
            func(data, extents, strides, indices, index + 1);
            indices[i] = indices[i] + 1;
        }
        std::cout << "]";
    }
}

TEST_CASE("typed_array mdarray tests")
{
    SECTION("row major strides")
    {
        std::vector<std::size_t> extents = { 2,6 };
        std::vector<std::size_t> strides = jsoncons::row_major::calculate_strides(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 6);
        CHECK(strides[1] == 1);

        CHECK(0 == jsoncons::get_offset(strides, {0, 0}));
        CHECK(1 == jsoncons::get_offset(strides, {0, 1}));
        CHECK(2 == jsoncons::get_offset(strides, {0, 2}));
        CHECK(6 == jsoncons::get_offset(strides, {1, 0}));
        CHECK(7 == jsoncons::get_offset(strides, {1, 1}));
        CHECK(8 == jsoncons::get_offset(strides, {1, 2}));
    }
    SECTION("column major strides")
    {
        std::vector<std::size_t> extents = { 2,6 };

        std::vector<std::size_t> strides = jsoncons::column_major::calculate_strides(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 1);
        CHECK(strides[1] == 2);

        CHECK(0 == jsoncons::get_offset(strides, {0, 0}));
        CHECK(1 == jsoncons::get_offset(strides, {1, 0}));
        CHECK(2 == jsoncons::get_offset(strides, {0, 1}));
        CHECK(3 == jsoncons::get_offset(strides, {1, 1}));
        CHECK(4 == jsoncons::get_offset(strides, {0, 2}));
        CHECK(5 == jsoncons::get_offset(strides, {1, 2}));
    }
    SECTION("print")
    {
        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};

        std::vector<std::size_t> extents = { 2,6 };
        std::vector<std::size_t> strides = jsoncons::row_major::calculate_strides(extents);

        std::vector<std::size_t> indices = {0,0};
        func(v, extents, strides, indices, 0);
    }
}

