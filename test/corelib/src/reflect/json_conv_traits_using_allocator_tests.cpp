// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/allocator_set.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/encode_json.hpp>
#include <sstream>
#include <vector>
#include <forward_list>
#include <list>
#include <utility>
#include <ctime>
#include <cstdint>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;
using cust_json = basic_json<char,sorted_policy,cust_allocator<char>>;

TEST_CASE("json_conv_traits using allocator tests")
{
    SECTION("std::map")
    {
        using char_allocator_type = mock_stateful_allocator<char>;
        using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"({"1" : "1", "2" : "2", "3" : "3"})";

        auto r1 = jsoncons::try_decode_json<cust_json>(aset, str);
        REQUIRE(r1);
        cust_json& j(*r1);
        REQUIRE(j.is_object());
        REQUIRE(3 == j.size());

        auto r2 = j.try_as<std::map<cust_string, cust_string, std::less<cust_string>, cust_allocator<std::pair<const cust_string,cust_string>>>>(aset);
        REQUIRE(r2);
    }
    SECTION("std::vector")
    {
        using char_allocator_type = mock_stateful_allocator<char>;
        using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1", "2", "3"])";

        auto r1 = jsoncons::try_decode_json<cust_json>(aset, str);
        REQUIRE(r1);
        cust_json& j(*r1);
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());

        auto r2 = j.try_as<std::vector<cust_string, cust_allocator<cust_string>>>(aset);
        REQUIRE(r2);
    }
    SECTION("std::forward_list")
    {
        using char_allocator_type = mock_stateful_allocator<char>;
        using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1", "2", "3"])";

        auto r1 = jsoncons::try_decode_json<cust_json>(aset, str);
        REQUIRE(r1);
        cust_json& j(*r1);
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());

        auto r2 = j.try_as<std::forward_list<cust_string, cust_allocator<cust_string>>>(aset);
        REQUIRE(r2);
    }
    SECTION("std::list")
    {
        using char_allocator_type = mock_stateful_allocator<char>;
        using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1", "2", "3"])";

        auto r1 = jsoncons::try_decode_json<cust_json>(aset, str);
        REQUIRE(r1);
        cust_json& j(*r1);
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());

        auto r2 = j.try_as<std::list<cust_string, cust_allocator<cust_string>>>(aset);
        REQUIRE(r2);
    }
    SECTION("basic_byte_string")
    {
        using byte_string_type = basic_byte_string<cust_allocator<uint8_t>>;

        cust_allocator<uint8_t> alloc(1);
        auto aset = make_alloc_set(alloc);

        cust_json j{byte_string{'H','e','l','l','o'}, aset.get_allocator()};
        REQUIRE(j.is<byte_string_type>());

        auto result = jsoncons::reflect::json_conv_traits<cust_json,byte_string_type>::try_as(aset, j);
        REQUIRE(result);
        auto r = j.try_as<byte_string_type>(aset);
        REQUIRE(r);
        CHECK(*r == *result);
        CHECK(j.as<byte_string_type>(aset) == *result);
        //std::cout << result.error() .message() << "\n\n";
    }
}

#endif
