// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <catch/catch.hpp>

using namespace jsoncons;

// https://tools.ietf.org/html/rfc4648#section-4 test vectors

void check_encode_base64(const std::vector<uint8_t>& input, const std::string& expected)
{
    std::string result;
    encode_base64(input.begin(),input.end(),result);
    REQUIRE(result.size() == expected.size());
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        CHECK(result[i] == expected[i]);
    }

    std::vector<uint8_t> output;
    decode_base64(result.begin(), result.end(), output);
    REQUIRE(output.size() == input.size());
    for (std::size_t i = 0; i < output.size(); ++i)
    {
        CHECK(output[i] == input[i]);
    }
}

void check_encode_base64url(const std::vector<uint8_t>& input, const std::string& expected)
{
    std::string result;
    encode_base64url(input.begin(),input.end(),result);
    REQUIRE(result.size() == expected.size());
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        CHECK(result[i] == expected[i]);
    }

    std::vector<uint8_t> output; 
    decode_base64url(result.begin(), result.end(), output);
    REQUIRE(output.size() == input.size());
    for (std::size_t i = 0; i < output.size(); ++i)
    {
        CHECK(output[i] == input[i]);
    }
}

void check_encode_base16(const std::vector<uint8_t>& input, const std::string& expected)
{
    std::string result;
    encode_base16(input.begin(),input.end(),result);
    REQUIRE(result.size() == expected.size());
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        CHECK(result[i] == expected[i]);
    }

    std::vector<uint8_t> output;
    decode_base16(result.begin(), result.end(), output);
    REQUIRE(output.size() == input.size());
    for (std::size_t i = 0; i < output.size(); ++i)
    {
        CHECK(output[i] == input[i]);
    }
}
TEST_CASE("test_base64_conversion")
{
    check_encode_base64({}, "");
    check_encode_base64({'f'}, "Zg==");
    check_encode_base64({'f','o'}, "Zm8=");
    check_encode_base64({'f','o','o'}, "Zm9v");
    check_encode_base64({'f','o','o','b'}, "Zm9vYg==");
    check_encode_base64({'f','o','o','b','a'}, "Zm9vYmE=");
    check_encode_base64({'f','o','o','b','a','r'}, "Zm9vYmFy");
}
 
TEST_CASE("test_base64url_conversion")
{
    check_encode_base64url({}, "");
    check_encode_base64url({'f'}, "Zg");
    check_encode_base64url({'f','o'}, "Zm8");
    check_encode_base64url({'f','o','o'}, "Zm9v");
    check_encode_base64url({'f','o','o','b'}, "Zm9vYg");
    check_encode_base64url({'f','o','o','b','a'}, "Zm9vYmE");
    check_encode_base64url({'f','o','o','b','a','r'}, "Zm9vYmFy");
}
 
TEST_CASE("test_base16_conversion")
{
    check_encode_base16({}, "");
    check_encode_base16({'f'}, "66");
    check_encode_base16({'f','o'}, "666F");
    check_encode_base16({'f','o','o'}, "666F6F");
    check_encode_base16({'f','o','o','b'}, "666F6F62");
    check_encode_base16({'f','o','o','b','a'}, "666F6F6261");
    check_encode_base16({'f','o','o','b','a','r'}, "666F6F626172");
}

TEST_CASE("byte_string_view constructors")
{
    SECTION("test 1")
    {
        std::vector<uint8_t> v = {'f','o','o','b','a','r'};
        byte_string_view bstr(v);
        CHECK(bstr[0] == 'f');
        CHECK(bstr[1] == 'o');
        CHECK(bstr[2] == 'o');
        CHECK(bstr[3] == 'b');
        CHECK(bstr[4] == 'a');
        CHECK(bstr[5] == 'r');

        byte_string_view copied(bstr);
        CHECK(copied == bstr);

        byte_string_view moved(std::move(bstr));
        CHECK(bstr.data() == nullptr);
        CHECK(bstr.size() == 0);

        REQUIRE(moved.size() == 6);
        CHECK(moved[0] == 'f');
        CHECK(moved[1] == 'o');
        CHECK(moved[2] == 'o');
        CHECK(moved[3] == 'b');
        CHECK(moved[4] == 'a');
        CHECK(moved[5] == 'r');
    }
}

TEST_CASE("byte_string_view iterators")
{
    SECTION("begin/end")
    {
        std::vector<uint8_t> v = {'f','o','o'};
        byte_string_view bstr(v);

        auto it = bstr.begin();
        REQUIRE(it != bstr.end());
        CHECK(*it++ == 'f');
        REQUIRE(it != bstr.end());
        CHECK(*it++ == 'o');
        REQUIRE(it != bstr.end());
        CHECK(*it++ == 'o');
        CHECK(it == bstr.end());
    }
}

