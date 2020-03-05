// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/detail/heap_only_string.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test_heap_only_string")
{
    std::string input = "Hello World";
    jsoncons::detail::heap_only_string_wrapper<char, std::allocator<char>> s(input.data(), input.size(), std::allocator<char>());

    //std::cout << s->c_str() << std::endl;
    CHECK(input == std::string(s.c_str()));
}

TEST_CASE("test_heap_only_string_wchar_t")
{
    std::wstring input = L"Hello World";
    jsoncons::detail::heap_only_string_wrapper<wchar_t, std::allocator<wchar_t>> s(input.data(), input.size(), std::allocator<wchar_t>());

    //std::wcout << s->c_str() << std::endl;

    CHECK(input == std::wstring(s.c_str()));
}

