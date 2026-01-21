// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/decode_toon.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

using namespace jsoncons;

TEST_CASE("decode_toon tests")
{
    SECTION("test 1")
    {
        std::string toon_str = R"([0]:)";
        auto result = toon::try_decode_toon<ojson>(toon_str);
        if (result)
        {
            std::cout << *result << "\n";
        }
    }
}
