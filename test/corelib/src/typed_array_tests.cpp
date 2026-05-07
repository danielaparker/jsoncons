// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/typed_array.hpp>
#include <iostream>
#include <catch/catch.hpp>

TEST_CASE("typed_array mdarray tests")
{
    SECTION("row major strides")
    {
        std::vector<std::size_t> extents = {2, 6};
        std::vector<std::size_t> strides = jsoncons::row_major_layout{}(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 6);
        CHECK(strides[1] == 1);
    }
    SECTION("column major strides")
    {
        std::vector<std::size_t> extents = {2, 6};

        std::vector<std::size_t> strides = jsoncons::column_major_layout{}(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 1);
        CHECK(strides[1] == 2);
    }
}
TEST_CASE("typed_array mdarray tests 2")
{
    SECTION("row major 2d")
    {
        // [[1 2 3 4 5 6 ][7 8 9 10 11 12 ]]

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,6 };

        jsoncons::mdarray_traverser<int> iter(v, extents);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{});
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("row major 3d")
    {
        // [[[1 2 ][3 4 ][5 6 ]][[7 8 ][9 10 ][11 12 ]]]

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,3,2 };

        jsoncons::mdarray_traverser<int> iter(v, extents);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{});
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("column major 2d")
    {
        std::string expected = "[[1 3 5 7 9 11 ][2 4 6 8 10 12 ]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 6};

        std::cout << "\nexpected: " << expected << "\n";
        jsoncons::mdarray_traverser<int> iter(v, extents, jsoncons::column_major_layout{});
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{});
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("column major 3d")
    {
        std::string expected = "[[[1 7 ][3 9 ][5 11 ]][[2 8 ][4 10 ][6 12 ]]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 3, 2};

        std::cout << "\nexpected: " << expected << "\n";
        jsoncons::mdarray_traverser<int> iter(v, extents, jsoncons::column_major_layout{});
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{});
        }
        std::cout << decoder.get_result() << "\n\n";
    }

}
