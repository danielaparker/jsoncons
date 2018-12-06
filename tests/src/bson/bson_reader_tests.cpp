// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::bson;

TEST_CASE("bson hello world")
{
    check_decode_bson({0x16,0x00,0x00,0x00, // total document size
                       0x02, // string
                       'h','e','l','l','o', 0x00, // field name 
                       0x06,0x00,0x00,0x00, // size of value
                       'w','o','r','l','d',0x00, // field value and null terminator
                       0x00 // end of document
                      },json::parse("{\"hello\":\"world\"}"));
}

