// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_filter.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;

TEST_CASE("test_small_string")
{
    json s("ABCD");
    CHECK(s.data_type() == jsoncons::data_type_tag::short_string_t);
    CHECK(s.as<std::string>() == std::string("ABCD"));

    json t(s);
    CHECK(t.data_type() == jsoncons::data_type_tag::short_string_t);
    CHECK(t.as<std::string>() == std::string("ABCD"));

    json q;
    q = s;
    CHECK(q.data_type() == jsoncons::data_type_tag::short_string_t);
    CHECK(q.as<std::string>() == std::string("ABCD"));
}


