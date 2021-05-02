// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/buffer_reader.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <fstream>

using namespace jsoncons;
 
TEST_CASE("buffer reader tests")
{
    SECTION("test 1")
    {
        json_buffer_reader<char,std::allocator<char>> reader(10,std::allocator<char>());
    }

}

