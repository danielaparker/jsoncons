// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

TEST_CASE("json(jsoncons::basic_string_view)")
{
    json::string_view_type sv("Hello world.");

    json j(sv);

    CHECK(j.as_string_view() == sv);
}

