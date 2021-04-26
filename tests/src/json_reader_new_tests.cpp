// Copyright 2021 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("Detect json encoding")
{
    SECTION("UTF16LE with lead surrogate")
    {
        std::vector<uint8_t> v = {'\"',0x00,0xD8,0x00,0xDB,0xFF,'\"',0x00};
        auto r = jsoncons::unicons::detect_json_encoding(v.data(),v.size());
        CHECK(r.kind == jsoncons::unicons::encoding_kind::utf16le);
        CHECK(r.ptr == v.data());
    }
    SECTION("UTF16BE with lead surrogate")
    {
        std::vector<uint8_t> v = {0x00,'\"',0x00,0xD8,0xFF,0xDB,0x00,'\"'};
        auto r = jsoncons::unicons::detect_json_encoding(v.data(),v.size());
        CHECK(r.kind == jsoncons::unicons::encoding_kind::utf16be);
        CHECK(r.ptr == v.data());
    }
}
