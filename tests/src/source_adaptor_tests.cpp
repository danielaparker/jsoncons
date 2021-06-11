// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/source_adaptor.hpp>
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
        json_buffer_reader<jsoncons::string_source<char>> reader{};
    }

}

