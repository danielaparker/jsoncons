// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/typed_array.hpp>
#include <iostream>
#include <catch/catch.hpp>

#if 0
TEST_CASE("typed_array mdarray tests")
{
    SECTION("row major strides")
    {
        std::vector<std::size_t> extents = { 2,6 };
        std::vector<std::size_t> strides = jsoncons::row_major_layout{}(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 6);
        CHECK(strides[1] == 1);

        CHECK(0 == jsoncons::get_offset(strides, std::vector<size_t>{0, 0}));
        CHECK(1 == jsoncons::get_offset(strides, std::vector<size_t>{0, 1}));
        CHECK(2 == jsoncons::get_offset(strides, std::vector<size_t>{0, 2}));
        CHECK(6 == jsoncons::get_offset(strides, std::vector<size_t>{1, 0}));
        CHECK(7 == jsoncons::get_offset(strides, std::vector<size_t>{1, 1}));
        CHECK(8 == jsoncons::get_offset(strides, std::vector<size_t>{1, 2}));
    }
    SECTION("column major strides")
    {
        std::vector<std::size_t> extents = { 2,6 };

        std::vector<std::size_t> strides = jsoncons::column_major_layout{}(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 1);
        CHECK(strides[1] == 2);

        CHECK(0 == jsoncons::get_offset(strides, std::vector<size_t>{0, 0}));
        CHECK(1 == jsoncons::get_offset(strides, std::vector<size_t>{1, 0}));
        CHECK(2 == jsoncons::get_offset(strides, std::vector<size_t>{0, 1}));
        CHECK(3 == jsoncons::get_offset(strides, std::vector<size_t>{1, 1}));
        CHECK(4 == jsoncons::get_offset(strides, std::vector<size_t>{0, 2}));
        CHECK(5 == jsoncons::get_offset(strides, std::vector<size_t>{1, 2}));
    }
    SECTION("print")
    {
        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};

        std::error_code ec;
        jsoncons::ser_context context{};

        jsoncons::json_decoder<jsoncons::json> decoder1;
        std::vector<std::size_t> extents = { 2,6 };
        jsoncons::to_json(jsoncons::span<const int>(v), extents, 
            jsoncons::typed_array_layout_kind::row_major, decoder1, 
            context, ec);
        std::cout << decoder1.get_result() << "\n\n";

        jsoncons::json_decoder<jsoncons::json> decoder2;
        jsoncons::to_json(jsoncons::span<const int>(v),
            extents, jsoncons::typed_array_layout_kind::column_major, decoder2, 
            context, ec);
        std::cout << decoder2.get_result() << "\n";
    }
}
#endif

TEST_CASE("typed_array mdarray tests 2")
{
    /*SECTION("row major strides")
    {
        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,6 };
        std::vector<std::size_t> strides = jsoncons::row_major_layout{}(extents);

        REQUIRE(extents.size() == strides.size());
        CHECK(strides[0] == 6);
        CHECK(strides[1] == 1);
        CHECK(0 == jsoncons::get_offset(strides, std::vector<size_t>{0, 0}));
        CHECK(6 == jsoncons::get_offset(strides, std::vector<size_t>{1, 0}));

        std::vector<std::size_t> indices1 = {0,0};

        std::vector<subarray_iterator> md(extents.size(), subarray_iterator{});
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            std::vector<std::size_t> indices(strides.size(), 0);

            md[i].extent = extents[i];
            md[i].offset = jsoncons::get_offset(strides, indices); 
            md[i].stride = strides[i]; 
        }

        std::size_t pos = 0;

        if (pos < md.back().extent)
        {
            std::size_t index = md.back().offset + pos*md.back().stride;
            std::cout << v[index] << " ";
        }
    }*/
    SECTION("row major 2d")
    {
        // [[1 2 3 4 5 6 ][7 8 9 10 11 12 ]]

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,6 };

        jsoncons::mdarray_traverser<int> iter(v, extents);
        for (std::size_t i = 0; i < 15; ++i)
        {
            iter.traverse();
        }
        std::cout << "\n\n";
    }
    SECTION("row major 3d")
    {
        // [[[1 2 ][3 4 ][5 6 ]][[7 8 ][9 10 ][11 12 ]]]

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,3,2 };

        jsoncons::mdarray_traverser<int> iter(v, extents);
        for (std::size_t i = 0; i < 15; ++i)
        {
            iter.traverse();
        }
        std::cout << "\n\n";
    }
    SECTION("column major 2d")
    {
        std::string expected = "[[1 3 5 7 9 11 ][2 4 6 8 10 12 ]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 6};

        std::cout << "\nexpected: " << expected << "\n";
        jsoncons::mdarray_traverser<int> iter(v, extents, jsoncons::column_major_layout{});
        for (std::size_t i = 0; i < 15; ++i)
        {
            iter.traverse();
        }
        std::cout << "\n\n";
    }
    SECTION("column major 3d")
    {
        std::string expected = "[[[1 7 ][3 9 ][5 11 ]][[2 8 ][4 10 ][6 12 ]]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 3, 2};

        std::cout << "\nexpected: " << expected << "\n";
        jsoncons::mdarray_traverser<int> iter(v, extents, jsoncons::column_major_layout{});
        for (std::size_t i = 0; i < 15; ++i)
        {
            iter.traverse();
        }
        std::cout << "\n\n";
    }
}
