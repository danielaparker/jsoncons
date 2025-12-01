// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/cursors/json_cursor.hpp>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("cursors::json_cursor constructor test")
{
    SECTION("default constructor")
    {
        cursors::json_cursor cursor{};
    }
}
