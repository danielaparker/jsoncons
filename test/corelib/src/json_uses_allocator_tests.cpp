// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/basic_json.hpp>
#include <jsoncons/json_uses_allocator.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("make_json_using_allocator tests")
{
    SECTION("test 1")
    {
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;
using cust_json = basic_json<char,sorted_policy,cust_allocator<char>>;

TEST_CASE("make_json_using_allocator with stateful allocator tests")
{
    SECTION("test 1")
    {
        cust_allocator<char> alloc(1);
        auto j = make_json_using_allocator<cust_json>(alloc,10);
        REQUIRE(j.is_uint64());
        CHECK(10 == j.as<int>());
    }
    SECTION("test 2")
    {
        cust_allocator<char> alloc(1);
        auto j = make_json_using_allocator<json>(alloc,10);
        REQUIRE(j.is_uint64());
        CHECK(10 == j.as<int>());
    }
}

#endif // JSONCONS_HAS_STATEFUL_ALLOCATOR
