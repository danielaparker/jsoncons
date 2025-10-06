// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <iostream>
#include <jsoncons/flat_hash_map.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("ujson tests")
{
    SECTION("test1")
    {
    }
    SECTION("test2")
    {       	
        jsoncons::flat_hash_map<std::string, json> my_hash_map;
        my_hash_map.emplace("key1", 10);
        my_hash_map["key2"] = json(20);

        auto it = my_hash_map.find("key1");
        std::cout << it->key() << ", " << it->value() << "\n";

        std::cout << my_hash_map["key2"] << "\n";
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>
template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using my_string = std::basic_string<char,std::char_traits<char>,MyScopedAllocator<char>>;

template <typename StringT>
struct MyHash
{
    std::size_t operator()(const StringT& s) const noexcept
    {
        const int p = 31;
        const int m = static_cast<int>(1e9) + 9;
        std::size_t hash_value = 0;
        std::size_t p_pow = 1;
        for (char c : s) {
            hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
            p_pow = (p_pow * p) % m;
        }
        return hash_value;   
    }
};

using my_flat_hash_map = jsoncons::flat_hash_map<my_string,json,MyHash<my_string>,std::equal_to<my_string>,MyScopedAllocator<key_value<my_string,json>>>;

TEST_CASE("cust_json.merge test with unordered_policy and stateful allocator")
{
    MyScopedAllocator<char> alloc(1);

    my_flat_hash_map m{alloc};
    m.emplace(my_string("key1",alloc), 10);
    m[my_string("key2", alloc)] = json(20);

    auto it = m.find(my_string(my_string("key2", alloc), alloc));
    std::cout << it->key() << ", " << it->value() << "\n";

    std::cout << m[my_string("key2", alloc)] << "\n";
}

#endif

