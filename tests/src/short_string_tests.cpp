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
    CHECK(s.structure_tag() == jsoncons::structure_tag_type::short_string_tag);
    CHECK(s.as<std::string>() == std::string("ABCD"));

    json t(s);
    CHECK(t.structure_tag() == jsoncons::structure_tag_type::short_string_tag);
    CHECK(t.as<std::string>() == std::string("ABCD"));

    json q;
    q = s;
    CHECK(q.structure_tag() == jsoncons::structure_tag_type::short_string_tag);
    CHECK(q.as<std::string>() == std::string("ABCD"));
}


