// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/detail/heap_string_box.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test heap_string_box char")
{
    std::string input = "Hello World";
    jsoncons::detail::heap_string_box<char, jsoncons::null_type, std::allocator<char>> s(input.data(), input.size(), 
        jsoncons::null_type(), std::allocator<char>());

    CHECK(input == std::string(s.c_str()));
    CHECK(s.length() == 11);
}

TEST_CASE("test heap_string_box wchar_t")
{
    std::wstring input = L"Hello World";
    jsoncons::detail::heap_string_box<wchar_t, jsoncons::null_type, std::allocator<wchar_t>> s(input.data(), input.size(), 
        jsoncons::null_type(), std::allocator<wchar_t>());

    CHECK(input == std::wstring(s.c_str()));
    CHECK(s.length() == 11);
}

TEST_CASE("test heap_string_box char uint8_t")
{
    std::string input = "Hello World";
    jsoncons::detail::heap_string_box<char, uint64_t, std::allocator<char>> s(input.data(), input.size(), 100, std::allocator<char>());

    CHECK(input == std::string(s.c_str()));
    CHECK(s.extra() == 100);
    CHECK(s.length() == 11);
}

TEST_CASE("test heap_string_box wchar_t uint8_t")
{
    std::wstring input = L"Hello World";
    jsoncons::detail::heap_string_box<wchar_t, uint64_t, std::allocator<wchar_t>> s(input.data(), input.size(), 100, std::allocator<wchar_t>());

    CHECK(input == std::wstring(s.c_str()));
    CHECK(s.extra() == 100);
    CHECK(s.length() == 11);
}

