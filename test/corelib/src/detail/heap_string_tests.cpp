// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/detail/heap_string.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

using heap_string_factory_type = jsoncons::detail::heap_string_factory<char, null_type, std::allocator<char>>;
using pointer = typename heap_string_factory_type::pointer;

TEST_CASE("heap_string test")
{
    std::string s("String too long for short string");

    pointer ptr = heap_string_factory_type::create(s.data(), s.length(), null_type(), std::allocator<char>());

    heap_string_factory_type::destroy(ptr);
}


