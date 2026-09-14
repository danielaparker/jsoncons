// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/utility/bigdec.hpp>
#include <catch/catch.hpp>

TEST_CASE("basic_bigdec tests")
{
    SECTION("0")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
    }
}

