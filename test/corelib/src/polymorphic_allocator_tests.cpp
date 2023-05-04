// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <common/FreeListAllocator.hpp>

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)

#include <memory_resource> 

using namespace jsoncons;

TEST_CASE("string polymorhic allocator tests")
{
    char buffer1[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
    std::pmr::polymorphic_allocator<char> alloc1(&pool1);

    char buffer2[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
    std::pmr::polymorphic_allocator<char> alloc2(&pool2);

    using pmr_json = jsoncons::pmr::json;
    using pmr_ojson = jsoncons::pmr::ojson;

    const char* long_string1 = "String too long for short string";

    CHECK_FALSE(traits_extension::is_stateless<std::pmr::polymorphic_allocator<char>>::value);
    CHECK_FALSE(alloc1 == alloc2);
    CHECK(alloc1 == alloc1);

    SECTION("construct string")
    {

        std::cout << "Polymorphic Allocator\n\n";

        std::cout << "propagate_on_container_copy_assignment: " << std::allocator_traits<std::pmr::polymorphic_allocator<char>>::propagate_on_container_copy_assignment::value << "\n";
        std::cout << "propagate_on_container_move_assignment: " << std::allocator_traits<std::pmr::polymorphic_allocator<char>>::propagate_on_container_move_assignment::value << "\n";
        std::cout << "propagate_on_container_swap: " << std::allocator_traits<std::pmr::polymorphic_allocator<char>>::propagate_on_container_swap::value << "\n";

        pmr_json j1(long_string1, alloc1);
        pmr_json j2(j1, alloc2);

        CHECK(j1.as<std::string>() == long_string1);
        CHECK(j2.as<std::string>() == long_string1);
        CHECK(j1.cast<pmr_json::long_string_storage>().get_allocator() == alloc1);
        CHECK(j2.cast<pmr_json::long_string_storage>().get_allocator() == alloc2);
        CHECK_FALSE(j1.cast<pmr_json::long_string_storage>().get_allocator() ==
            j2.cast<pmr_json::long_string_storage>().get_allocator());
    }
}

TEST_CASE("Test polymorhic allocator")
{
    char buffer1[1024] = {}; // a small buffer1 on the stack
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
    std::pmr::polymorphic_allocator<char> alloc1(&pool1);

    char buffer2[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
    std::pmr::polymorphic_allocator<char> alloc2(&pool2);

    using pmr_json = jsoncons::pmr::json;
    using pmr_ojson = jsoncons::pmr::ojson;

    const char* long_string = "String too long for short string";

    CHECK_FALSE(traits_extension::is_stateless<std::pmr::polymorphic_allocator<char>>::value);

    SECTION("construct string")
    {
        pmr_json j(long_string, alloc1);

        CHECK(j.as<std::string>() == long_string);
    }

    SECTION("try_emplace json")
    {
        pmr_json j(json_object_arg, alloc1);

        std::pmr::string key1{"foo", alloc1};
        std::pmr::string key2{"bar", alloc1};

        j.try_emplace(key1, pmr_json{});
        j.try_emplace(std::move(key2), long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("try_emplace ojson")
    {
        pmr_ojson j(json_object_arg, alloc1);

        std::pmr::string key1{"foo", alloc1};
        std::pmr::string key2{"bar", alloc1};

        j.try_emplace(key1, pmr_ojson{});
        j.try_emplace(std::move(key2), long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_ojson{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("insert_or_assign json")
    {
        pmr_json j(json_object_arg, alloc1);

        j.insert_or_assign("foo", pmr_json{});
        j.insert_or_assign("bar", long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("insert_or_assign ojson")
    {
        pmr_ojson j(json_object_arg, alloc1);

        j.insert_or_assign("foo", pmr_ojson{});
        j.insert_or_assign("bar", long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_ojson{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("emplace_back")
    {
        pmr_json j(json_array_arg, alloc1);
        j.emplace_back(1);
        j.emplace_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("push_back")
    {
        pmr_json j(json_array_arg, alloc1);
        j.push_back(1);
        j.push_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("insert")
    {
        pmr_json j(json_array_arg, alloc1);

        j.insert(j.array_range().end(), pmr_json{});
        j.insert(j.array_range().end(), long_string);

        CHECK(j.size() == 2);
        CHECK(j[0] == pmr_json{});
        CHECK(j[1].as_string_view() == long_string);
    }
/*

    SECTION("parse")
    {
        FreeListAllocator<char> alloc2(true); 

        std::string s = long_string;
        std::string input = "\"" + s + "\"";

        json_decoder<pmr_json,FreeListAllocator<char>> decoder(result_allocator_arg, alloc1, alloc2);
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
    */
}
#endif // namespace JSONCONS_HAS_2017


