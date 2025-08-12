// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/utility/more_type_traits.hpp>
#include <jsoncons/conversion.hpp>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("convert into string")
{
    std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};

    SECTION("from byte_string into string")
    {
        std::string expected = "Zm9vYmFy";

        std::string s;
        bytes_to_string(bytes.begin(), bytes.end(), semantic_tag::base64url, s);
        
        CHECK(expected == s);
    }
    SECTION("from byte string into wstring")
    {
        std::wstring expected = L"Zm9vYmFy";

        std::wstring s;
        bytes_to_string(bytes.begin(), bytes.end(), semantic_tag::base64url, s);

        CHECK(expected == s);
    }
}

TEST_CASE("convert into list-like")
{
    SECTION("from string")
    {
        std::vector<uint8_t> expected = {'f','o','o','b','a','r'};

        jsoncons::string_view sv("Zm9vYmFy");
        std::vector<uint8_t> v;
        auto r = string_to_bytes(sv.begin(), sv.end(), semantic_tag::base64url, v);
        REQUIRE(r.ec == conv_errc{});
        
        CHECK(expected == v);
    }
}

