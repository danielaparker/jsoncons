// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 
#endif
#include <catch/catch.hpp>

namespace extension_traits = jsoncons::extension_traits;

TEST_CASE("extension_traits tests")
{
    SECTION("is_propagating_allocator")
    {
        CHECK_FALSE(jsoncons::extension_traits::is_propagating_allocator<std::allocator<char>>::value);
#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
        CHECK(jsoncons::extension_traits::is_propagating_allocator<std::pmr::polymorphic_allocator<char>>::value);
#endif
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/FreeListAllocator.hpp>
template<typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<FreeListAllocator<T>>;

TEST_CASE("extension_traits tests is_propagating_allocator")
{
    SECTION("is_propagating_allocator")
    {
        CHECK_FALSE(jsoncons::extension_traits::is_propagating_allocator<FreeListAllocator<char>>::value);
        CHECK(jsoncons::extension_traits::is_propagating_allocator<MyScopedAllocator<char>>::value);
    }
}
#endif

TEST_CASE("extension_traits function object tests")
{
    SECTION("is_unary_function_object")
    {
        CHECK_FALSE(extension_traits::is_unary_function_object<std::string,int>::value);
    }
    SECTION("is_unary_function_object_exact")
    {
        CHECK_FALSE(extension_traits::is_unary_function_object_exact<std::string, int, int>::value);
    }
    SECTION("is_binary_function_object")
    {
        CHECK_FALSE(extension_traits::is_binary_function_object<std::string, int, int>::value);
    }
    SECTION("is_unary_function_object_exact")
    {
        CHECK_FALSE(extension_traits::is_binary_function_object_exact<std::string, int, int, int>::value);
    }
}

