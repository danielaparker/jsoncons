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
        std::vector<std::size_t> extents = { 2,6 };
        std::vector<std::size_t> strides = jsoncons::row_major_layout::calculate_strides(extents);

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

        std::vector<std::size_t> strides = jsoncons::column_major_layout::calculate_strides(extents);

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

