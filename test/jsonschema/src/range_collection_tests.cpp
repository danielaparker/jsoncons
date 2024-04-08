// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <jsoncons/byte_string.hpp>

#include <catch/catch.hpp>
#include <fstream>
#include <iostream>
#include <regex>

class range
{
    std::size_t start_;
    std::size_t end_;
public:
    range()
        : start_(0), end_(0)
    {
    }

    range(std::size_t start, std::size_t end)
        : start_(start), end_(end)
    {
    }
    
    std::size_t start() const
    {
        return start_;
    }

    std::size_t end() const
    {
        return end_;
    }
    
    bool contains(std::size_t index) const
    {
        return index >= start_ && index < end_; 
    }
};

class range_collection
{
    std::vector<range> ranges_;
public:    
    range_collection()
    {
    }
    
    std::size_t size() const
    {
        return ranges_.size();
    }

    range operator[](std::size_t index) const
    {
        return ranges_[index];
    }
    
    void push_back(range index_range)
    {
        ranges_.push_back(index_range);
    }
    
    void append(const range_collection& collection)
    {
        std::size_t length = collection.size();
        for (std::size_t i = 0; i < length; ++i)
        {
            ranges_.push_back(collection[i]);
        }
    }

    bool contains(std::size_t index) const
    {
        bool found = false;
        std::size_t length = ranges_.size();
        for (std::size_t i = 0; i < length && !found; ++i)
        {
            if (ranges_[i].contains(index))
            {
                found = true;
            }
        }
        return found;
    }
};

TEST_CASE("jsonschema range collection tests")
{
    SECTION("test 1")
    {
        range_collection ranges;
        ranges.push_back(range{0,5});
        ranges.push_back(range{10,15});
        ranges.push_back(range{7,8});
        
        CHECK(ranges.contains(0));
        CHECK(ranges.contains(1));
        CHECK(ranges.contains(2));
        CHECK(ranges.contains(3));
        CHECK(ranges.contains(4));
        CHECK_FALSE(ranges.contains(5));
        CHECK_FALSE(ranges.contains(6));
        CHECK(ranges.contains(7));
        CHECK_FALSE(ranges.contains(8));
        CHECK_FALSE(ranges.contains(9));
        CHECK(ranges.contains(10));
        CHECK(ranges.contains(11));
        CHECK(ranges.contains(12));
        CHECK(ranges.contains(13));
        CHECK(ranges.contains(14));
    }
}

