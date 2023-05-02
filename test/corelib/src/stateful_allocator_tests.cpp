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
TEST_CASE("test_string_allocation")
{

    FreeListAllocator<char> alloc(true); 

    using my_json = basic_json<char,sorted_policy,FreeListAllocator<char>>;

    SECTION("construct")
    {
        {
            my_json j("String too long for short string", alloc);
        }
        //std::cout << "Allocate count = " << a_pool.allocate_count_ 
        //          << ", construct count = " << a_pool.construct_count_ 
        //          << ", destroy count = " << a_pool.destroy_count_ 
        //          << ", deallocate count = " << a_pool.deallocate_count_ << std::endl;
    }

    SECTION("construct")
    {
        my_json j("String too long for short string", alloc);
       //CHECK(a_pool.allocate_count_ == a_pool.deallocate_count_);
       // CHECK(a_pool.construct_count_ == a_pool.destroy_count_);
    }

    SECTION("emplace_back")
    {
        my_json j(json_array_arg, semantic_tag::none, alloc);
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

        json_decoder<my_json,FreeListAllocator<char>> decoder(result_allocator_arg, alloc, alloc2);
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

