// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <catch/catch.hpp>

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/FreeListAllocator.hpp>

using namespace jsoncons;

template<typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<FreeListAllocator<T>>;

using custom_json = jsoncons::basic_json<char, jsoncons::sorted_policy, MyScopedAllocator<char>>;
using custom_string = std::basic_string<char, std::char_traits<char>, MyScopedAllocator<char>>;

TEST_CASE("scoped allocator adaptor basic_json tests")
{

    MyScopedAllocator<char> alloc1(1);

    using custom_json = basic_json<char,sorted_policy,MyScopedAllocator<char>>;
    using custom_string = std::basic_string<char,std::char_traits<char>,MyScopedAllocator<char>>;

    const char* long_string = "String too long for short string";

    CHECK_FALSE(extension_traits::is_stateless<MyScopedAllocator<char>>::value);

    SECTION("construct from string")
    {
        custom_json j(long_string, alloc1);
        CHECK(j.as<std::string>() == long_string);
    }

    SECTION("try_emplace")
    {
        custom_json j(json_object_arg, alloc1);

        custom_string key1{"foo", alloc1};
        custom_string key2{"bar", alloc1};

        j.try_emplace(key1, custom_json{});
        j.try_emplace(std::move(key2), long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == custom_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("insert_or_assign")
    {
        custom_json j(json_object_arg, alloc1);

        j.insert_or_assign("foo", custom_json{});
        j.insert_or_assign("bar", long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == custom_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("emplace_back")
    {
        custom_json j(json_array_arg, alloc1);
        j.emplace_back(1);
        j.emplace_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("push_back")
    {
        custom_json j(json_array_arg, alloc1);
        j.push_back(1);
        j.push_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("insert")
    {
        custom_json j(json_array_arg, alloc1);

        j.insert(j.array_range().end(), custom_json{});
        j.insert(j.array_range().end(), long_string);

        CHECK(j.size() == 2);
        CHECK(j[0] == custom_json{});
        CHECK(j[1].as_string_view() == long_string);
    }
}

TEST_CASE("scoped allocator adaptor parse tests")
{
    using custom_json = basic_json<char,sorted_policy,MyScopedAllocator<char>>;
    using custom_string = std::basic_string<char,std::char_traits<char>,MyScopedAllocator<char>>;

    CHECK_FALSE(extension_traits::is_stateless<MyScopedAllocator<char>>::value);

    MyScopedAllocator<char> alloc1(1); 
    MyScopedAllocator<char> alloc2(2); 

    custom_string data = custom_string(R"(

{"foo" : [{"short" : "bar",
          "long" : "string to long for short string", 
          "false" : false, 
          "true" : true,
          "null" : null,
          "integer" : 10,
          "double" : 1000.1}]
}
    )", alloc2);

    SECTION("parse")
    {
        json_decoder<custom_json,MyScopedAllocator<char>> decoder(alloc1, alloc2);
        JSONCONS_TRY
        {
            json_string_reader reader(data,decoder);
            reader.read_next();
        }
        JSONCONS_CATCH (const std::exception& ex)
        {
            std::cout << ex.what() << "\n\n";
        }
        CHECK(decoder.is_valid());
        auto j = decoder.get_result();

        CHECK(j.contains("foo"));

        custom_json& a = j.at("foo");
        CHECK(a.size() == 1);
        custom_json& b = a[0];

        CHECK(b.at("double").as<double>() == Approx(1000.1).epsilon(0.001));
        CHECK(b.at("integer").as<int>() == 10);
        CHECK(b.at("null") == custom_json::null());
        CHECK(b.at("false") == custom_json(false));
        CHECK(b.at("true") == custom_json(true));
        CHECK(b.at("short") == custom_json("bar", alloc1));
        CHECK(b.at("long") == custom_json("string to long for short string", alloc1));
    }

}

#endif

