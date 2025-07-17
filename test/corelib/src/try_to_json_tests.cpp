// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/decode_json.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/utility/more_type_traits.hpp>
#include <vector>

#include <catch/catch.hpp>

TEST_CASE("cursor try_to_json")
{
    SECTION("single")
    {
        std::string s = R"("foo")"; 
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<jsoncons::json>(cursor);
        REQUIRE(res);
        std::cout << res.value() << "\n";
    }
    SECTION("array")
    {
        std::string s = R"([false, 1, "foo"])";
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<jsoncons::ojson>(cursor);
        REQUIRE(res);
        jsoncons::ojson& j(res.value());
        REQUIRE(j.is_array());
        //std::cout << j << "\n";
    }
    SECTION("array of arrays")
    {
        std::string s = R"([[null, false, true], [1.5, 123456]])";
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<jsoncons::json>(cursor);
        REQUIRE(res);
        jsoncons::json& j(res.value());
        REQUIRE(j.is_array());
        //std::cout << j << "\n";
    }

    SECTION("object")
    {
        std::string s = R"(
{
    "short" : "bar",
    "long" : "string to long for short string"
}
    )";
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<jsoncons::ojson>(cursor);
        REQUIRE(res);
        jsoncons::ojson& j(res.value());
        REQUIRE(j.is_object());
        std::cout << res.value() << "\n";
    }
    SECTION("object with nested containers")
    {
        std::string s = R"(
{"foo" : [{"short" : "bar",
          "long" : "string to long for short string", 
          "false" : false, 
          "true" : true,
          "null" : null,
          "integer" : 10,
          "double" : 1000.1}]
}
    )";
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<jsoncons::json>(cursor);
        REQUIRE(res);
        jsoncons::json& j(res.value());
        REQUIRE(j.is_object());
        std::cout << res.value() << "\n";
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/free_list_allocator.hpp>
#include <jsoncons/json_type.hpp>

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using my_json = jsoncons::basic_json<char,jsoncons::sorted_policy,MyScopedAllocator<char>>;

using vec_allocator_type = typename std::allocator_traits<MyScopedAllocator<char>>:: template rebind_alloc<my_json>;

TEST_CASE("try_to_json with temp allocator")
{
    MyScopedAllocator<char> temp_alloc(1);

    /*SECTION("test1")
    {
        my_json j{jsoncons::json_array_arg, temp_alloc};
        j.emplace_back(my_json(jsoncons::json_array_arg, temp_alloc));
    }

    SECTION("test1a")
    {
        my_json j{jsoncons::json_array_arg, temp_alloc};
        j.emplace_back("foo");
        std::cout << j << "\n";
    }

    SECTION("test2")
    {
        my_json j{jsoncons::json_object_arg, temp_alloc};
        j.try_emplace("key", jsoncons::json_array_arg);
    }*/

    SECTION("test 3")
    {
        using my_allocator_type = std::scoped_allocator_adaptor<mock_stateful_allocator<char>>;
        using my_json = jsoncons::basic_json<char, jsoncons::sorted_policy, my_allocator_type>;
        using vec_allocator_type = typename std::allocator_traits<my_allocator_type>:: template rebind_alloc<my_json>;

        std::cout << "is convertible: " << std::is_convertible_v<my_allocator_type, my_allocator_type> << "\n";
        std::cout << "uses allocator: " << std::uses_allocator_v<my_json, my_allocator_type> << "\n";
        std::cout << "array uses allocator: " << std::uses_allocator_v<my_json::array::value_type, my_json::array::allocator_type> << "\n";

        my_allocator_type alloc{1};
        std::vector<my_json, vec_allocator_type> v(alloc);
        v.emplace_back(jsoncons::json_array_arg);
    }

    /*SECTION("test 4")
    {
        std::vector<std::basic_string<char>, MyScopedAllocator<std::basic_string<char>>> v(temp_alloc);
        v.emplace_back();
    }

    SECTION("array")
    {
        std::string s = R"([false, 1, "foo"])";
        jsoncons::json_string_cursor cursor(s);
        auto res = jsoncons::try_to_json<my_json>(jsoncons::make_alloc_set(temp_alloc,temp_alloc), cursor);
        REQUIRE(res);
        my_json& j(res.value());
        REQUIRE(j.is_array());
    }*/
}

#endif
