// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_filter.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;

TEST_CASE("test_small_string")
{
    json s("ABCD");
    CHECK(s.type() == jsoncons::storage_type::short_string_val);
    CHECK(s.as<std::string>() == std::string("ABCD"));

    json t(s);
    CHECK(t.type() == jsoncons::storage_type::short_string_val);
    CHECK(t.as<std::string>() == std::string("ABCD"));

    json q;
    q = s;
    CHECK(q.type() == jsoncons::storage_type::short_string_val);
    CHECK(q.as<std::string>() == std::string("ABCD"));
}


