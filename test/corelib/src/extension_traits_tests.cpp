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
#include <scoped_allocator>
#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)
#include <memory_resource> 
#endif
#include <catch/catch.hpp>

template<typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<FreeListAllocator<T>>;

TEST_CASE("extension_traits tests")
{
    SECTION("is_propagating_allocator")
    {
        CHECK_FALSE(jsoncons::extension_traits::is_propagating_allocator<std::allocator<char>>::value);
        CHECK_FALSE(jsoncons::extension_traits::is_propagating_allocator<FreeListAllocator<char>>::value);
        CHECK(jsoncons::extension_traits::is_propagating_allocator<MyScopedAllocator<char>>::value);
#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)
        CHECK(jsoncons::extension_traits::is_propagating_allocator<std::pmr::polymorphic_allocator<char>>::value);
#endif
    }
}

