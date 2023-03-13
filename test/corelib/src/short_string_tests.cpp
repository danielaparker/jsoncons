// Copyright 2013-2023 Daniel Parker
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
    CHECK(s.storage_kind() == jsoncons::json_storage_kind::short_string_value);
    CHECK(s.as<std::string>() == std::string("ABCD"));

    json t(s);
    CHECK(t.storage_kind() == jsoncons::json_storage_kind::short_string_value);
    CHECK(t.as<std::string>() == std::string("ABCD"));

    json q;
    q = s;
    CHECK(q.storage_kind() == jsoncons::json_storage_kind::short_string_value);
    CHECK(q.as<std::string>() == std::string("ABCD"));
}


