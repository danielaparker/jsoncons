// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
//#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace 
{
    class MyIterator
    {
        const char* p_;
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = const char*; 
        using reference = const char&;

        MyIterator(const char* p)
            : p_(p)
        {
        }

        reference operator*() const
        {
            return *p_;
        }

        pointer operator->() const 
        {
            return p_;
        }

        MyIterator& operator++()
        {
            ++p_;
            return *this;
        }

        MyIterator operator++(int) 
        {
            MyIterator temp(*this);
            ++*this;
            return temp;
        }

        bool operator!=(const MyIterator& rhs) const
        {
            return p_ != rhs.p_;
        }
    };

} // namespace

TEST_CASE("encode decode csv source")
{
    using cpp_type = std::vector<std::tuple<std::string,int>>;
    std::string input = "\"a\",1\n\"b\",2";
    csv::csv_options options;
    options.mapping(csv::mapping_kind::n_rows)
           .assume_header(false);

    SECTION("from string")
    {
        cpp_type v = csv::decode_csv<cpp_type>(input, options);
        REQUIRE(v.size() == 2);
        CHECK(std::get<0>(v[0]) == "a");
        CHECK(std::get<1>(v[0]) == 1);
        CHECK(std::get<0>(v[1]) == "b");
        CHECK(std::get<1>(v[1]) == 2);

        std::string s2;
        csv::encode_csv(v, s2, options);

        json j1 = csv::decode_csv<json>(input);
        json j2 = csv::decode_csv<json>(s2);

        CHECK(j2 == j1);

        json j3 = csv::decode_csv<json>(s2.begin(), s2.end());
        CHECK(j3 == j1);
    }

    SECTION("from stream")
    {
        std::stringstream is(input);
        cpp_type v = csv::decode_csv<cpp_type>(is, options);
        REQUIRE(v.size() == 2);
        CHECK(std::get<0>(v[0]) == "a");
        CHECK(std::get<1>(v[0]) == 1);
        CHECK(std::get<0>(v[1]) == "b");
        CHECK(std::get<1>(v[1]) == 2);

        std::stringstream ss2;
        csv::encode_csv(v, ss2, options);

        json j1 = csv::decode_csv<json>(input);

        json j2 = csv::decode_csv<json>(ss2);
        CHECK(j2 == j1);
    }

    SECTION("from iterator")
    {
        cpp_type v = csv::decode_csv<cpp_type>(input.begin(), input.end(), options);
        REQUIRE(v.size() == 2);
        CHECK(std::get<0>(v[0]) == "a");
        CHECK(std::get<1>(v[0]) == 1);
        CHECK(std::get<0>(v[1]) == "b");
        CHECK(std::get<1>(v[1]) == 2);

        std::stringstream ss2;
        csv::encode_csv(v, ss2, options);

        json j1 = csv::decode_csv<json>(input);
        json j2 = csv::decode_csv<json>(ss2);

        CHECK(j2 == j1);
    }

    SECTION("from custom iterator")
    {
        MyIterator it(input.data());
        MyIterator end(input.data() + input.length());

        cpp_type v = csv::decode_csv<cpp_type>(it, end, options);
        REQUIRE(v.size() == 2);
        CHECK(std::get<0>(v[0]) == "a");
        CHECK(std::get<1>(v[0]) == 1);
        CHECK(std::get<0>(v[1]) == "b");
        CHECK(std::get<1>(v[1]) == 2);

        std::stringstream ss2;
        csv::encode_csv(v, ss2, options);

        json j1 = csv::decode_csv<json>(input);
        json j2 = csv::decode_csv<json>(ss2);

        CHECK(j2 == j1);
    }
}


