// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <common/FreeListAllocator.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <catch/catch.hpp>
#include <scoped_allocator>

#if (defined(__GNUC__) && (__GNUC__ == 4)) && (defined(__GNUC__) && __GNUC_MINOR__ < 9)
// gcc 4.8 basic_string doesn't satisfy C++11 allocator requirements
// and gcc doesn't support allocators with no default constructor
#else

template<typename T>
using ScopedTestAllocator = std::scoped_allocator_adaptor<FreeListAllocator<T>>;

using xjson = jsoncons::basic_json<char, jsoncons::sorted_policy, ScopedTestAllocator<char>>;


TEST_CASE("scoped_allocator_adaptor tests")
{
    ScopedTestAllocator<char> alloc(true);

    SECTION("construct")
    {
        xjson(1, jsoncons::semantic_tag::none);
    }
}

#endif

