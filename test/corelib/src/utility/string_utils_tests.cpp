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
        std::string str = "Hello World";
        CHECK(jsoncons::starts_with(str, "Hel"));
        CHECK(jsoncons::starts_with(str, ""));
        CHECK_FALSE(jsoncons::starts_with(str, "Hello World 2"));
    }
    SECTION("ends_with")
    {
        std::string str = "Hello World";
        CHECK(jsoncons::ends_with(str, "rld"));
        CHECK(jsoncons::ends_with(str, ""));
        CHECK_FALSE(jsoncons::ends_with(str, "Hello World 2"));
    }
    SECTION("strip")
    {
        CHECK(jsoncons::string_view{} == jsoncons::strip(jsoncons::string_view{}));
        CHECK(jsoncons::string_view{"Hello World"} == jsoncons::strip(jsoncons::string_view{" Hello World "}));
        CHECK(jsoncons::string_view{"Hello World"} == jsoncons::strip(jsoncons::string_view{"Hello World "}));
        CHECK(jsoncons::string_view{"Hello World"} == jsoncons::strip(std::string{" Hello World"}));
        CHECK(jsoncons::string_view{"Hello World"} == jsoncons::strip(std::string{"   Hello World   "}));
        CHECK(jsoncons::string_view{"Hello World"} == jsoncons::strip(std::string{"   Hello World   "}));
    }
}

