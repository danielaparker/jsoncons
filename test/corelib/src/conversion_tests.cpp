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
        value_converter<jsoncons::string_view, std::vector<uint8_t>> converter;

        std::vector<uint8_t> expected = {'f','o','o','b','a','r'};

        std::error_code ec;
        std::vector<uint8_t> v = converter.convert(jsoncons::string_view("Zm9vYmFy"), semantic_tag::base64url, ec);
        REQUIRE(!ec); 
        
        CHECK(expected == v);
    }
}

