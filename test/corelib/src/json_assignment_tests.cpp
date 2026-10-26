// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>

#include <ctime>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 

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

    std::vector<uint8_t> bstr = { 'H','e','l','l','o' };
    std::vector<uint8_t> bstr2 = { 'W','o','r','l','d' };

    SECTION("long string to long string assignment")
    {
        jsoncons::pmr::json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{long_string2, alloc2};
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
        jsoncons::pmr::json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("byte string to byte string assignment")
    {
        jsoncons::pmr::json j1{byte_string_arg, bstr, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, bstr.data(), bstr.data()+bstr.size());
        CHECK(it != last1);

        jsoncons::pmr::json j2{byte_string_arg, bstr2, jsoncons::semantic_tag::none, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, bstr2.data(), bstr2.data()+bstr2.size());
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        it = std::search(buffer1, last1, bstr.data(), bstr.data()+bstr.size());
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        it = std::search(buffer2, last2, bstr2.data(), bstr2.data()+bstr2.size());
        CHECK(j1 == j2);
    }

    SECTION("byte string to byte string move assignment")
    {
        jsoncons::pmr::json j1{byte_string_arg, bstr, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, bstr.data(), bstr.data()+bstr.size());
        CHECK(it != last1);

        jsoncons::pmr::json j2{byte_string_arg, bstr2, jsoncons::semantic_tag::none, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        it = std::search(buffer2, last2, bstr2.data(), bstr2.data()+bstr2.size());
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("array to array assignment")
    {
        jsoncons::pmr::json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{jsoncons::json_array_arg, alloc2};
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
        jsoncons::pmr::json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{jsoncons::json_array_arg, alloc2};
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
        jsoncons::pmr::json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_key1, long_key1_end);
        it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{jsoncons::json_object_arg, alloc2};
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
        jsoncons::pmr::json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_key1, long_key1_end);
        it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.try_emplace(long_key2, long_string2);
        it = std::search(buffer2, last2, long_key2, long_key2_end);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource());
        REQUIRE(&pool1 == j2.get_allocator().resource());
    }

    SECTION("long string to number assignment")
    {
        jsoncons::pmr::json j1{10};

        jsoncons::pmr::json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        auto it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(j1.get_allocator() == std::pmr::polymorphic_allocator<char>{});
        CHECK(j1 == j2);
    }

    SECTION("number to long string assignment")
    {
        jsoncons::pmr::json j1{10};

        jsoncons::pmr::json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        auto it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j2 = j1;
        CHECK(j2.is_number());
    }

    SECTION("long string to number move assignment")
    {
        jsoncons::pmr::json j1{10};

        jsoncons::pmr::json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        auto it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = std::move(j2);
        REQUIRE(&pool2 == j1.get_allocator().resource()); 
        CHECK(j1.as<std::string>() == long_string2);
        CHECK(j2.is_number());
    }

    SECTION("number to long string move assignment")
    {
        jsoncons::pmr::json j1{10};

        jsoncons::pmr::json j2{long_string2, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource()); 
        auto it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j2 = std::move(j1);
        CHECK(j2.is_number());
        CHECK(j1.is_string());
        REQUIRE(&pool2 == j1.get_allocator().resource()); 
    }

    SECTION("object to array assignment")
    {
        jsoncons::pmr::json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(&pool2 == j2.get_allocator().resource());
        j2.try_emplace(long_key2, long_string2);
        it = std::search(buffer2, last2, long_key2, long_key2_end);
        it = std::search(buffer2, last2, long_string2, long_string2_end);
        CHECK(it != last2);

        j1 = j2;
        REQUIRE(&pool1 == j1.get_allocator().resource());
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(&pool2 == j2.get_allocator().resource());
        CHECK(j1 == j2);
    }
}

#endif // polymorphic_allocator

TEST_CASE("json assignment between a value and a value it contains")
{
    const std::string long_string = "String too long for short string";

    SECTION("long string member to object assignment")
    {
        json j(json_object_arg, {{"a", long_string}});
        j = j.at("a");
        CHECK(long_string == j.as<std::string>());
    }

    SECTION("number member to object assignment")
    {
        json j(json_object_arg, {{"a", 10}});
        j = j.at("a");
        CHECK(10 == j.as<int>());
    }

    SECTION("array member to object assignment")
    {
        json j(json_object_arg, {{"a", json(json_array_arg, {1, 2, json(long_string)})}});
        j = j.at("a");
        CHECK(json(json_array_arg, {1, 2, json(long_string)}) == j);
    }

    SECTION("object member to object assignment")
    {
        json j(json_object_arg, {{"a", json(json_object_arg, {{"b", long_string}})}});
        j = j.at("a");
        CHECK(json(json_object_arg, {{"b", long_string}}) == j);
    }

    SECTION("object to its long string member assignment")
    {
        json j(json_object_arg, {{"a", long_string}});
        json expected(j);
        j.at("a") = j;
        CHECK(expected == j.at("a"));
    }

    SECTION("array to its long string element assignment")
    {
        json j(json_array_arg, {json(long_string)});
        json expected(j);
        j[0] = j;
        CHECK(expected == j[0]);
    }

    SECTION("array element to array assignment")
    {
        json j(json_array_arg, {json(json_array_arg, {1, 2, json(long_string)}), 4, 5});
        j = j[0];
        CHECK(json(json_array_arg, {1, 2, json(long_string)}) == j);
    }

    SECTION("long string element to array assignment")
    {
        json j(json_array_arg);
        j.push_back(long_string);
        j = j[0];
        CHECK(j.as<std::string>() == long_string);
    }

    SECTION("json reference to element to array assignment")
    {
        json j(json_array_arg);
        j.push_back(long_string);
        json ref(json_ptr_arg, &j[0]);
        j = ref;
        CHECK(j.as<std::string>() == long_string);
    }
}

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

TEST_CASE("json assignment with scoped allocator")
{
    using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<char>>;
    using cust_json = jsoncons::basic_json<char,jsoncons::sorted_policy,cust_allocator>;

    cust_allocator alloc1(1);
    cust_allocator alloc2(2);

    const char* long_key1 = "Key too long for short string";
    const char* long_key2 = "Another key too long for short string";
    const char* long_string1 = "String too long for short string";
    const char* long_string2 = "Another string too long for short string";

    std::vector<uint8_t> bstr = { 'H','e','l','l','o' };
    std::vector<uint8_t> bstr2 = { 'W','o','r','l','d' };

    SECTION("long string to long string assignment")
    {
        cust_json j1{long_string1, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{long_string2, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 
        
        j1 = j2;
        REQUIRE(alloc1 == j1.get_allocator());

        j2 = j1;
        REQUIRE(alloc2 == j2.get_allocator());
    }

    SECTION("long string to long string move assignment")
    {
        cust_json j1{long_string1, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{long_string2, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 

        j1 = std::move(j2);
        REQUIRE(alloc2 == j1.get_allocator());
        REQUIRE(alloc1 == j2.get_allocator());
    }

    SECTION("byte string to byte string assignment")
    {
        cust_json j1{byte_string_arg, bstr, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{byte_string_arg, bstr2, jsoncons::semantic_tag::none, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 

        j1 = j2;
        REQUIRE(alloc1 == j1.get_allocator());

        j2 = j1;
        REQUIRE(alloc2 == j2.get_allocator());
    }

    SECTION("byte string to byte string move assignment")
    {
        cust_json j1{byte_string_arg, bstr, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{byte_string_arg, bstr2, jsoncons::semantic_tag::none, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 

        j1 = std::move(j2);
        REQUIRE(alloc2 == j1.get_allocator());
        REQUIRE(alloc1 == j2.get_allocator());
    }

    SECTION("array to array assignment")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.push_back(long_string1); 

        cust_json j2{jsoncons::json_array_arg, alloc2};
        REQUIRE(alloc2 == j2.get_allocator());
        j2.push_back(long_string2);

        j1 = j2;
        REQUIRE(alloc1 == j1.get_allocator());

        j2 = j1;
        REQUIRE(alloc2 == j2.get_allocator());
    }

    SECTION("array to array move assignment")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.push_back(long_string1); 

        cust_json j2{jsoncons::json_array_arg, alloc2};
        REQUIRE(alloc2 == j2.get_allocator());
        j2.push_back(long_string2);

        j1 = std::move(j2);
        REQUIRE(alloc2 == j1.get_allocator());
        REQUIRE(alloc1 == j2.get_allocator());
    }

    SECTION("object to object assignment")
    {
        cust_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.insert_or_assign(long_key1, long_string1); 

        cust_json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(alloc2 == j2.get_allocator());
        j2.try_emplace(long_key2, long_string2);

        j1 = j2;
        REQUIRE(alloc1 == j1.get_allocator());

        j2 = j1;
        REQUIRE(alloc2 == j2.get_allocator());
    }

    SECTION("object to object move assignment")
    {
        cust_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.insert_or_assign(long_key1, long_string1); 

        cust_json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(alloc2 == j2.get_allocator());
        j2.try_emplace(long_key2, long_string2);

        j1 = std::move(j2);
        REQUIRE(alloc2 == j1.get_allocator());
        REQUIRE(alloc1 == j2.get_allocator());
    }

    SECTION("long string to number assignment")
    {
        cust_json j1{10};

        cust_json j2{long_string2, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 

        j1 = j2;
        REQUIRE(alloc2 == j1.get_allocator());
        CHECK(j1 == j2);
    }

    SECTION("number to long string assignment")
    {
        cust_json j1{10};

        cust_json j2{long_string2, alloc2};
        REQUIRE(alloc2 == j2.get_allocator()); 

        j2 = j1;
        CHECK(j2.is_number());
    }

    SECTION("object to array assignment")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.push_back(long_string1); 

        cust_json j2{jsoncons::json_object_arg, alloc2};
        REQUIRE(alloc2 == j2.get_allocator());
        j2.try_emplace(long_key2, long_string2);

        j1 = j2;
        REQUIRE(alloc1 == j1.get_allocator());
        CHECK(j1 == j2);

        j2 = j1;
        REQUIRE(alloc2 == j2.get_allocator());
        CHECK(j1 == j2);
    }

    SECTION("json reference to number assignment")
    {
        cust_json j1{10};

        cust_json j2{jsoncons::json_array_arg, alloc2};
        j2.push_back(long_string2);
        cust_json ref{jsoncons::json_ptr_arg, &j2};

        j1 = ref;
        REQUIRE(alloc2 == j1.get_allocator());
        CHECK(j1 == j2);
    }

    SECTION("json reference to json reference assignment")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        j1.push_back(long_string1);
        cust_json j2{jsoncons::json_array_arg, alloc2};
        j2.push_back(long_string2);
        cust_json ref{jsoncons::const_json_ptr_arg, &j2};

        cust_json j3{jsoncons::const_json_ptr_arg, &j1};
        j3 = ref;
        REQUIRE(alloc2 == j3.get_allocator());
        CHECK(j3 == j2);
    }

    SECTION("json reference to itself assignment")
    {
        cust_json j1{long_string1, alloc1};
        cust_json ref{jsoncons::json_ptr_arg, &j1};

        j1 = ref;
        REQUIRE(alloc1 == j1.get_allocator());
        CHECK(j1.as<std::string>() == long_string1);
    }
}

#endif // scoped_allocator

