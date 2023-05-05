// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/extension_traits.hpp>
#include <jsoncons/value_converter.hpp>
#include <vector>

using namespace jsoncons;

TEST_CASE("convert into string")
{
    std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};

    SECTION("from byte_string into string")
    {
        value_converter<byte_string_view,std::string> converter;

        std::string expected = "Zm9vYmFy";

        std::error_code ec;
        std::string s = converter.convert(byte_string_view(bytes), semantic_tag::base64url, ec);
        REQUIRE(!ec); 
        
        CHECK(s == expected);
    }
    SECTION("from byte string into wstring")
    {
        value_converter<byte_string_view,std::wstring> converter;

        std::wstring expected = L"Zm9vYmFy";

        std::error_code ec;
        std::wstring s = converter.convert(byte_string_view(bytes), semantic_tag::base64url, ec);
        REQUIRE(!ec);

        CHECK(s == expected);
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
        
        CHECK(v == expected);
    }
    SECTION("from wstring")
    {
        value_converter<jsoncons::basic_string_view<wchar_t>,std::vector<uint8_t>> converter;

        std::vector<uint8_t> expected = { 'f','o','o','b','a','r' };

        std::error_code ec;
        std::vector<uint8_t> v = converter.convert(jsoncons::wstring_view(L"Zm9vYmFy"), semantic_tag::base64url, ec);
        REQUIRE(!ec);

        CHECK(v == expected);
    }
}

