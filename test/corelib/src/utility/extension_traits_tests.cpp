// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>

#include <cstddef>
#include <ctime>
#include <sstream>
#include <utility>
#include <vector>
#include <catch/catch.hpp>

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 
#endif

namespace ext_traits = jsoncons::ext_traits;

TEST_CASE("ext_traits tests")
{
    SECTION("is_propagating_allocator")
    {
        CHECK_FALSE(jsoncons::ext_traits::is_propagating_allocator<std::allocator<char>>::value);
#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
        CHECK(jsoncons::ext_traits::is_propagating_allocator<std::pmr::polymorphic_allocator<char>>::value);
#endif
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>
template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

TEST_CASE("ext_traits tests is_propagating_allocator")
{
    SECTION("is_propagating_allocator")
    {
        CHECK_FALSE(jsoncons::ext_traits::is_propagating_allocator<mock_stateful_allocator<char>>::value);
        CHECK(jsoncons::ext_traits::is_propagating_allocator<MyScopedAllocator<char>>::value);
    }
}
#endif

TEST_CASE("ext_traits function object tests")
{
    SECTION("is_function_object_1")
    {
        CHECK_FALSE(ext_traits::is_function_object_1<std::string,int>::value);
    }
    SECTION("is_function_object_1_exact")
    {
        CHECK_FALSE(ext_traits::is_function_object_1_exact<std::string, int, int>::value);
    }
    SECTION("is_function_object_2")
    {
        CHECK_FALSE(ext_traits::is_function_object_2<std::string, int, int>::value);
    }
    SECTION("is_function_object_1_exact")
    {
        CHECK_FALSE(ext_traits::is_function_object_2_exact<std::string, int, int, int>::value);
    }
}

