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

    const char* long_key = "Key too long for short string";
    const char* long_key_end = long_key + strlen(long_key);
    const char* long_string = "String too long for short string";
    const char* long_string_end = long_string + strlen(long_string);
    const char* another_long_string = "Another string too long for short string";
    const char* another_long_string_end = another_long_string + strlen(another_long_string);

    std::vector<uint8_t> byte_string = { 'H','e','l','l','o' };

    SECTION("long string to long string assignment")
    {
        pmr_json j1{long_string, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string, long_string_end);
        CHECK(it != last1);

        pmr_json j2{another_long_string, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, another_long_string, another_long_string_end);
        CHECK(it != last2);
        
        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, another_long_string, another_long_string_end);
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, another_long_string, another_long_string_end);
        CHECK(j1 == j2);
    }

    SECTION("long string to long string move assignment")
    {
        pmr_json j1{long_string, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string, long_string_end);
        CHECK(it != last1);

        pmr_json j2{another_long_string, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, another_long_string, another_long_string_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }
}

#endif

