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

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)
#include <memory_resource> 
using namespace jsoncons;

using pmr_json = jsoncons::pmr::json;
using pmr_ojson = jsoncons::pmr::ojson;

TEST_CASE("heap_string_box constructors")
{
    char buffer1[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
    std::pmr::polymorphic_allocator<char> alloc1(&pool1);

    char buffer2[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
    std::pmr::polymorphic_allocator<char> alloc2(&pool2);

    jsoncons::string_view long_string = "String too long for short string";
    jsoncons::string_view empty_string = "";

    using custom_string_box = jsoncons::detail::heap_string_box<char, null_type, std::pmr::polymorphic_allocator<char>>;

    SECTION("copy")
    {
        custom_string_box s1(long_string.data(), long_string.size(), jsoncons::null_type(), alloc1);

        custom_string_box s2(s1);
        CHECK(s2.c_str() == long_string);
        CHECK(s2.get_allocator() == alloc1);
        CHECK(jsoncons::string_view(s1.c_str()) == long_string);

        custom_string_box s3(s2, alloc2);
        CHECK(s3.c_str() == long_string);
        CHECK_FALSE(s3.get_allocator() == alloc1);
        CHECK(s3.get_allocator() == alloc2);

        custom_string_box s4(s3, alloc2);
        CHECK(s4.c_str() == long_string);
        CHECK(jsoncons::string_view(s3.c_str()) == long_string);
        CHECK(s4.get_allocator() == alloc2);
    }

    SECTION("move")
    {
        custom_string_box s1(long_string.data(), long_string.size(), jsoncons::null_type(), alloc1);

        custom_string_box s2(std::move(s1));
        CHECK(s2.c_str() == long_string);
        CHECK(s2.get_allocator() == alloc1);
        //CHECK(jsoncons::string_view(s1.c_str()) == empty_string);  // s3 is moved

        custom_string_box s3(std::move(s2), alloc2);
        CHECK(s3.c_str() == long_string);
        CHECK_FALSE(s3.get_allocator() == alloc1);
        CHECK(s3.get_allocator() == alloc2);

        custom_string_box s4(std::move(s3), alloc2);
        CHECK(s4.c_str() == long_string);
        //CHECK(jsoncons::string_view(s3.c_str()) == empty_string); // s3 is moved
        CHECK(s4.get_allocator() == alloc2);
    }

}

#endif

