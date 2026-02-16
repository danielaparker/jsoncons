// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/utility/string_utils.hpp>
#include <catch/catch.hpp>

TEST_CASE("string_utils tests")
{
    SECTION("starts_with")
    {
        CHECK(jsoncons::starts_with("Hello World", "Hel"));
        CHECK(jsoncons::starts_with("Hello World", ""));
        CHECK_FALSE(jsoncons::starts_with("Hello World", "Hello World 2"));
    }
    SECTION("ends_with")
    {
        CHECK(jsoncons::ends_with("Hello World", "rld"));
        CHECK(jsoncons::ends_with("Hello World", ""));
        CHECK_FALSE(jsoncons::ends_with("Hello World", "Hello World 2"));
    }
    SECTION("strip")
    {
        CHECK(std::string_view{} == jsoncons::strip(""));
        CHECK(std::string_view{"Hello World"} == jsoncons::strip(" Hello World "));
        CHECK(std::string_view{"Hello World"} == jsoncons::strip("Hello World "));
        CHECK(std::string_view{"Hello World"} == jsoncons::strip(" Hello World"));
        CHECK(std::string_view{"Hello World"} == jsoncons::strip("   Hello World   "));
        CHECK(std::string_view{"Hello World"} == jsoncons::strip("   Hello World   "));
    }
}

