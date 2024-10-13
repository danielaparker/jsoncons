// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;


#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 
using namespace jsoncons;

using pmr_json = jsoncons::pmr::json;
using pmr_ojson = jsoncons::pmr::ojson;

TEST_CASE("json assignment with pmr allocator")
{
    char buffer1[1024] = {}; // a small buffer on the stack
    char* last1 = buffer1 + sizeof(buffer1);
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
    std::pmr::polymorphic_allocator<char> alloc1(&pool1);

    char buffer2[1024] = {}; // another small buffer on the stack
    char* last2 = buffer2 + sizeof(buffer2);
    std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
    std::pmr::polymorphic_allocator<char> alloc2(&pool2);

    const char* long_key1 = "Key too long for short string";
    const char* long_key1_end = long_key1 + strlen(long_key1);
    const char* long_key2 = "Another key too long for short string";
    const char* long_key2_end = long_key2 + strlen(long_key2);
    const char* long_string1 = "String too long for short string";
    const char* long_string1_end = long_string1 + strlen(long_string1);
    const char* long_string2 = "Another string too long for short string";
    const char* long_string2_end = long_string2 + strlen(long_string2);

    std::vector<uint8_t> byte_string = { 'H','e','l','l','o' };
    std::vector<uint8_t> byte_string2 = { 'W','o','r','l','d' };

    SECTION("long string to long string assignment")
    {
        pmr_json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);
        
        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, long_string2, long_string2_end);
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(j1 == j2);
    }

    SECTION("long string to long string move assignment")
    {
        pmr_json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("byte string to byte string assignment")
    {
        pmr_json j1{byte_string_arg, byte_string, semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, byte_string.data(), byte_string.data()+byte_string.size());
        CHECK(it != last1);

        pmr_json j2{byte_string_arg, byte_string2, semantic_tag::none, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, byte_string2.data(), byte_string2.data()+byte_string2.size());
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, byte_string.data(), byte_string.data()+byte_string.size());
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, byte_string2.data(), byte_string2.data()+byte_string2.size());
        CHECK(j1 == j2);
    }

    SECTION("byte string to byte string move assignment")
    {
        pmr_json j1{byte_string_arg, byte_string, semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, byte_string.data(), byte_string.data()+byte_string.size());
        CHECK(it != last1);

        pmr_json j2{byte_string_arg, byte_string2, semantic_tag::none, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, byte_string2.data(), byte_string2.data()+byte_string2.size());
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("array to array assignment")
    {
        pmr_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{jsoncons::json_array_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.push_back(long_string2);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, long_string2, long_string2_end);
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(j1 == j2);
    }

    SECTION("array to array move assignment")
    {
        pmr_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{jsoncons::json_array_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.push_back(long_string2);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("object to object assignment")
    {
        pmr_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_key1, long_key1_end);
        it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.try_emplace(long_key2, long_string2);
        it = std::search(buffer2, last2, long_key2, long_key2_end);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, long_string2, long_string2_end);
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(j1 == j2);
    }

    SECTION("object to object move assignment")
    {
        pmr_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_key1, long_key1_end);
        it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        pmr_json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.try_emplace(long_key2, long_string2);
        it = std::search(buffer2, last2, long_key2, long_key2_end);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }
}

#endif

