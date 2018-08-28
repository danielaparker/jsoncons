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
#include <boost/optional.hpp>

using namespace jsoncons;

TEST_CASE("test_small_string")
{
    json s("ABCD");
    CHECK(s.major_type() == jsoncons::json_major_type::short_string_t);
    CHECK(s.as<std::string>() == std::string("ABCD"));

    json t(s);
    CHECK(t.major_type() == jsoncons::json_major_type::short_string_t);
    CHECK(t.as<std::string>() == std::string("ABCD"));

    json q;
    q = s;
    CHECK(q.major_type() == jsoncons::json_major_type::short_string_t);
    CHECK(q.as<std::string>() == std::string("ABCD"));
}


