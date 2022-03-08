// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/converter.hpp>
#include <vector>

using namespace jsoncons;

TEST_CASE("convert into list-like")
{
    SECTION("from string")
    {
        converter<std::vector<uint8_t>> convert;

        std::vector<uint8_t> expected = {'f','o','o','b','a','r'};

        std::error_code ec;
        std::vector<uint8_t> v = convert.from(jsoncons::string_view("Zm9vYmFy"), semantic_tag::base64url, ec);
        REQUIRE(!ec); 
        
        CHECK(v == expected);
    }
    SECTION("from wstring")
    {
        converter<std::vector<uint8_t>> convert;

        std::vector<uint8_t> expected = { 'f','o','o','b','a','r' };

        std::error_code ec;
        std::vector<uint8_t> v = convert.from(jsoncons::wstring_view(L"Zm9vYmFy"), semantic_tag::base64url, ec);
        REQUIRE(!ec);

        CHECK(v == expected);
    }
}

TEST_CASE("convert into string")
{
    std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};

    SECTION("from byte_string into string")
    {
        converter<std::string> convert;

        std::string expected = "Zm9vYmFy";

        std::error_code ec;
        std::string s = convert.from(byte_string_view(bytes), semantic_tag::base64url, ec);
        REQUIRE(!ec); 
        
        CHECK(s == expected);
    }
    SECTION("from byte string into wstring")
    {
        converter<std::wstring> convert;

        std::wstring expected = L"Zm9vYmFy";

        std::error_code ec;
        std::wstring s = convert.from(byte_string_view(bytes), semantic_tag::base64url, ec);
        REQUIRE(!ec);

        CHECK(s == expected);
    }
}

