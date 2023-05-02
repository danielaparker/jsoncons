// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <common/FreeListAllocator.hpp>

#if defined(JSONCONS_HAS_2017)

#include <memory_resource> 

using namespace jsoncons;

TEST_CASE("Test polymorhic allocator")
{
    char buffer[1024] = {}; // a small buffer on the stack
    std::fill_n(std::begin(buffer), std::size(buffer) - 1, '_');
    std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
    std::pmr::polymorphic_allocator<char> alloc(&pool);

    using pmr_json = jsoncons::pmr::json;

    SECTION("construct string")
    {
        pmr_json j("String too long for short string", alloc);

        CHECK(j.as<std::string>() == "String too long for short string");
    }

    SECTION("emplace_back")
    {
        pmr_json j(json_array_arg, alloc);
        j.emplace_back(1);
        j.emplace_back("String too long for short string");

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == "String too long for short string");
    }

    SECTION("push_back")
    {
        pmr_json j(json_array_arg, alloc);
        j.push_back(1);
        j.push_back("String too long for short string");

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == "String too long for short string");
    }
/*
    SECTION("emplace_back")
    {
        pmr_json j(json_array_arg, semantic_tag::none, alloc);
        j.emplace_back("String too long for short string", alloc);

        std::cout << j << "\n";


        //CHECK(a_pool.allocate_count_ == a_pool.deallocate_count_);
        // CHECK(a_pool.construct_count_ == a_pool.destroy_count_);
    }

    SECTION("parse")
    {
        FreeListAllocator<char> alloc2(true); 

        std::string s = "String too long for short string";
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


