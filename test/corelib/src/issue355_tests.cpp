// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <list>

using namespace jsoncons;

TEST_CASE("issue 355 test")
{
    jsoncons::json someObject;
    jsoncons::json::array someArray(4);
    someArray[0] = 0;
    someArray[1] = 1;
    someArray[2] = 2;
    someArray[3] = 3;

    // This will end-up making a copy the array, which I did not expect.
    someObject.insert_or_assign("someKey", std::move(someArray));
}

