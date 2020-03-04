// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/random_access_iterator_wrapper.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("random_access_iterator_wrapper test")
{
    std::vector<int> v = {1,2,3,4};

    typedef jsoncons::random_access_iterator_wrapper<std::vector<int>::iterator> iterator;
    typedef jsoncons::random_access_iterator_wrapper<std::vector<int>::const_iterator> const_iterator;

    SECTION("Test 1")
    {
        iterator it(v.begin());
        iterator last(v.end());
    }
}

