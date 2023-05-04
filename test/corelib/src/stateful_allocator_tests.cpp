// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <common/FreeListAllocator.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR)
TEST_CASE("test stateful allocator")
{
    FreeListAllocator<char> alloc1(true); 

    using custom_json = basic_json<char,sorted_policy,FreeListAllocator<char>>;
    using custom_string = std::basic_string<char,std::char_traits<char>,FreeListAllocator<char>>;

    const char* long_string = "String too long for short string";

    CHECK_FALSE(traits_extension::is_stateless<FreeListAllocator<char>>::value);

    SECTION("construct from string")
    {
        custom_json j(long_string, alloc1);
        CHECK(j.as<std::string>() == long_string);

        std::cout << "Stateful Allocator\n\n";
        std::cout << "propagate_on_container_copy_assignment: " << std::allocator_traits<FreeListAllocator<char>>::propagate_on_container_copy_assignment::value << "\n";
        std::cout << "propagate_on_container_move_assignment: " << std::allocator_traits<FreeListAllocator<char>>::propagate_on_container_move_assignment::value << "\n";
        std::cout << "propagate_on_container_swap: " << std::allocator_traits<FreeListAllocator<char>>::propagate_on_container_swap::value << "\n";
    }

    SECTION("try_emplace")
    {
        custom_json j(json_object_arg, alloc1);

        custom_string key1{"foo", alloc1};
        custom_string key2{"bar", alloc1};

        j.try_emplace(key1, custom_json{});
        j.try_emplace(std::move(key2), long_string, alloc1);

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
        j.emplace_back(long_string, alloc1);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("push_back")
    {
        custom_json j(json_array_arg, alloc1);
        j.push_back(1);
        j.push_back(custom_json(long_string, alloc1));

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("insert")
    {
        custom_json j(json_array_arg, alloc1);

        j.insert(j.array_range().end(), custom_json{});
        j.insert(j.array_range().end(), custom_json(long_string, alloc1));

        CHECK(j.size() == 2);
        CHECK(j[0] == custom_json{});
        CHECK(j[1].as_string_view() == long_string);
    }

    SECTION("parse")
    {
        FreeListAllocator<char> alloc2(true); 

        std::string s = long_string;
        std::string input = "\"" + s + "\"";

        json_decoder<custom_json,FreeListAllocator<char>> decoder(result_allocator_arg, alloc1, alloc2);
        JSONCONS_TRY
        {
            json_string_reader reader(input,decoder);
            reader.read_next();
        }
        JSONCONS_CATCH (const std::exception&)
        {
        }
        CHECK(decoder.is_valid());
        auto j = decoder.get_result();
        CHECK(j.as<std::string>() == s);
    }

}
#endif

