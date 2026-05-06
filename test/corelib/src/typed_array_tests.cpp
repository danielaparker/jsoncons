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

template <typename ValueType>
struct subarray_iterator
{
    std::size_t extent;
    std::size_t start;
    std::size_t end{0};
    std::size_t stride{0};
    std::size_t index{0};
};

template <typename ValueType>
class array_iterator
{
    jsoncons::span<ValueType> data_;
    std::vector<subarray_iterator<ValueType>> iterators_;
public:
    template <typename Layout= jsoncons::row_major_layout>
    array_iterator(jsoncons::span<ValueType> data, const std::vector<std::size_t>& extents,
        Layout layout = Layout())
        : data_{data}, iterators_(extents.size(), subarray_iterator<ValueType>{})
    {
        std::vector<std::size_t> strides = layout(extents);
        std::vector<std::size_t> indices(strides.size(), 0);
        for (std::size_t i = 0; i < strides.size(); ++i)
        {
            std::vector<std::size_t> to_indices(strides.size(), 0);
            for (std::size_t j = 0; j <= i; ++j)
            {
                to_indices[j] = extents[j];
            }

            iterators_[i].extent = extents[i];
            iterators_[i].stride = strides[i];
            iterators_[i].start = jsoncons::get_offset(strides, indices);
            iterators_[i].index = iterators_[i].start;
            iterators_[i].end = iterators_[i].start + strides[i]*extents[i];
        }
        std::cout << "[";
    }

    void print()
    {
        std::cout << data_[iterators_.back().index] << " "; 
    }

    void next()
    {
        if (iterators_.empty())
        {
            return;
        }
        std::size_t index = iterators_.size() - 1;
        if (iterators_[index].index + iterators_[index].stride < iterators_[index].end)
        {
            iterators_[index].index += iterators_[index].stride;
        }
        else 
        {
            std::cout << "]";
            bool done = false;
            while (index > 0 && !done)
            {
                --index;
                if (iterators_[index].index + iterators_[index].stride < iterators_[index].end)
                {
                    iterators_[index].index += iterators_[index].stride;
                    for (std::size_t i = index+1; i < iterators_.size(); ++i)
                    {
                        iterators_[i].start = iterators_[i-1].index;
                        iterators_[i].index = iterators_[i].start;
                        iterators_[i].end = iterators_[i].index + iterators_[i].stride*iterators_[i].extent;
                    }
                    done = true;
                }
            }
        }
    }
};

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

        array_iterator<int> iter(v, extents);
        for (std::size_t i = 0; i < 12; ++i)
        {
            iter.print();
            iter.next();
        }
        std::cout << "\n\n";
    }
    SECTION("row major 3d")
    {
        // [[[1 2 ][3 4 ][5 6 ]][[7 8 ][9 10 ][11 12 ]]]

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,3,2 };

        array_iterator<int> iter(v, extents);
        for (std::size_t i = 0; i < 12; ++i)
        {
            iter.print();
            iter.next();
        }
        std::cout << "\n\n";
    }
    SECTION("column major 2d")
    {
        std::string expected = "[[1 3 5 7 9 11 ][2 4 6 8 10 12 ]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 6};

        std::cout << "\nexpected: " << expected << "\n";
        array_iterator<int> iter(v, extents, jsoncons::column_major_layout{});
        for (std::size_t i = 0; i < 12; ++i)
        {
            iter.print();
            iter.next();
        }
        std::cout << "\n\n";
    }
    SECTION("column major 3d")
    {
        std::string expected = "[[[1 7 ][3 9 ][5 11 ]][[2 8 ][4 10 ][6 12 ]]]";

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 3, 2};

        std::cout << "\nexpected: " << expected << "\n";
        array_iterator<int> iter(v, extents, jsoncons::column_major_layout{});
        for (std::size_t i = 0; i < 12; ++i)
        {
            iter.print();
            iter.next();
        }
        std::cout << "\n\n";
    }
}
