// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <common/FreeListAllocator.hpp>

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)

#include <memory_resource> 

using namespace jsoncons;

TEST_CASE("Test polymorhic allocator")
{
    char buffer[1024] = {}; // a small buffer on the stack
    std::fill_n(std::begin(buffer), std::size(buffer) - 1, '_');
    std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
    std::pmr::polymorphic_allocator<char> alloc(&pool);

    using pmr_json = jsoncons::pmr::json;

    const char* long_string = "String too long for short string";

    CHECK_FALSE(traits_extension::is_stateless<std::pmr::polymorphic_allocator<char>>::value);

    SECTION("construct string")
    {
        pmr_json j(long_string, alloc);

        CHECK(j.as<std::string>() == long_string);
    }

    SECTION("try_emplace")
    {
        pmr_json j(json_object_arg, alloc);

        std::pmr::string key1{"foo", alloc};
        std::pmr::string key2{"bar", alloc};

        j.try_emplace(key1, pmr_json{});
        j.try_emplace(std::move(key2), long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("insert_or_assign")
    {
        pmr_json j(json_object_arg, alloc);

        j.insert_or_assign("foo", pmr_json{});
        j.insert_or_assign("bar", long_string);

        CHECK(j.size() == 2);
        CHECK(j.at("foo") == pmr_json{});
        CHECK(j.at("bar").as_string_view() == long_string);
    }

    SECTION("emplace_back")
    {
        pmr_json j(json_array_arg, alloc);
        j.emplace_back(1);
        j.emplace_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }

    SECTION("push_back")
    {
        pmr_json j(json_array_arg, alloc);
        j.push_back(1);
        j.push_back(long_string);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == long_string);
    }
/*

    SECTION("parse")
    {
        FreeListAllocator<char> alloc2(true); 

        std::string s = long_string;
        std::string input = "\"" + s + "\"";

        json_decoder<pmr_json,FreeListAllocator<char>> decoder(result_allocator_arg, alloc, alloc2);
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


