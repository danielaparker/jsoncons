// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 

TEST_CASE("json constructor with pmr allocator")
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
    const char* long_string1 = "String too long for short string";
    const char* long_string1_end = long_string1 + strlen(long_string1);
    const char* long_string2 = "Another string too long for short string";
    const char* long_string2_end = long_string2 + strlen(long_string2);

    std::vector<uint8_t> jsoncons::byte_string1 = { 'H','e','l','l','o' };

    SECTION("long string copy constructor")
    {
        jsoncons::pmr::json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{j1};
        REQUIRE_FALSE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{j1, alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("long string move constructor")
    {
        jsoncons::pmr::json j1{long_string1, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{std::move(j1)};
        REQUIRE(&pool1 == j2.get_allocator().resource()); 
        it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j3{std::move(j2), alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("byte string copy constructor")
    {
        jsoncons::pmr::json j1{byte_string_arg, jsoncons::byte_string1, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, jsoncons::byte_string1.data(), jsoncons::byte_string1.data()+byte_string1.size());
        CHECK(it != last1);

        jsoncons::pmr::json j2{j1};
        REQUIRE_FALSE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{j1, alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("byte string move constructor")
    {
        jsoncons::pmr::json j1{byte_string_arg, jsoncons::byte_string1, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource()); 
        auto it = std::search(buffer1, last1, jsoncons::byte_string1.data(), jsoncons::byte_string1.data()+byte_string1.size());
        CHECK(it != last1);

        jsoncons::pmr::json j2{std::move(j1)};
        REQUIRE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{std::move(j2), alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("array copy constructor")
    {
        jsoncons::pmr::json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        j1.push_back(long_string2);
        auto it = std::search(buffer1, last1, long_string2, long_string2_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{j1};
        REQUIRE_FALSE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{j1, alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("array move constructor")
    {
        jsoncons::pmr::json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.push_back(long_string1); 
        j1.push_back(long_string2);
        auto it = std::search(buffer1, last1, long_string2, long_string2_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{std::move(j1)};
        REQUIRE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{std::move(j2), alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("object copy constructor")
    {
        jsoncons::pmr::json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{j1};
        REQUIRE_FALSE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{j1, alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_key1, long_key1_end);
        CHECK(it != last1);
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("object move constructor")
    {
        jsoncons::pmr::json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{std::move(j1)};
        REQUIRE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{std::move(j2), alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_key1, long_key1_end);
        CHECK(it != last1);
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }

    SECTION("empty object with given allocator")
    {
        jsoncons::pmr::json j1{alloc1};
        REQUIRE(j1.is_object());
        REQUIRE(&pool1 == j1.get_allocator().resource());
    }

    SECTION("object move constructor")
    {
        jsoncons::pmr::json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(&pool1 == j1.get_allocator().resource());
        j1.insert_or_assign(long_key1, long_string1); 
        auto it = std::search(buffer1, last1, long_string1, long_string1_end);
        CHECK(it != last1);

        jsoncons::pmr::json j2{std::move(j1)};
        REQUIRE(&pool1 == j2.get_allocator().resource()); 

        jsoncons::pmr::json j3{std::move(j2), alloc2};
        REQUIRE(&pool2 == j3.get_allocator().resource()); 
        it = std::search(buffer2, last2, long_key1, long_key1_end);
        CHECK(it != last1);
        it = std::search(buffer2, last2, long_string1, long_string1_end);
        CHECK(it != last1);
    }
}

#endif // polymorphic_allocator

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

TEST_CASE("json constructor with scoped_allocator")
{
    using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<char>>;
    using cust_json = jsoncons::basic_json<char,jsoncons::sorted_policy,cust_allocator>;
    //using cust_ojson = jsoncons::basic_json<char,ordered_policy,cust_allocator>;

    cust_allocator alloc1(1);
    cust_allocator alloc2(2);

    REQUIRE(std::allocator_traits<mock_stateful_allocator<char>>::propagate_on_container_swap::value);
    REQUIRE(std::allocator_traits<mock_stateful_allocator<char>>::propagate_on_container_move_assignment::value);
    REQUIRE_FALSE(std::allocator_traits<mock_stateful_allocator<char>>::propagate_on_container_copy_assignment::value);

    const char* long_key1 = "Key too long for short string";
    const char* long_string1 = "String too long for short string";
    const char* long_string2 = "Another string too long for short string";

    std::vector<uint8_t> jsoncons::byte_string1 = { 'H','e','l','l','o' };
    
    SECTION("long string copy constructor")
    {
        cust_json j1{long_string1, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{j1};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{j1, alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
        REQUIRE_FALSE(alloc1 == j3.get_allocator()); 
    }

    SECTION("long string move constructor")
    {
        cust_json j1{long_string1, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
        REQUIRE_FALSE(alloc1 == j3.get_allocator()); 
    }

    SECTION("byte string copy constructor")
    {
        cust_json j1{byte_string_arg, jsoncons::byte_string1, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{j1};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{j1, alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
        REQUIRE_FALSE(alloc1 == j3.get_allocator()); 
    }

    SECTION("byte string move constructor")
    {
        cust_json j1{byte_string_arg, jsoncons::byte_string1, jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator()); 

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }

    SECTION("array copy constructor")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.push_back(long_string1); 
        j1.push_back(long_string2);

        cust_json j2{j1};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{j1, alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }

    SECTION("array move constructor")
    {
        cust_json j1{jsoncons::json_array_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.push_back(long_string1); 
        j1.push_back(long_string2);

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }

    SECTION("object copy constructor")
    {
        cust_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.insert_or_assign(long_key1, long_string1); 

        cust_json j2{j1};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{j1, alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }

    SECTION("object move constructor")
    {
        cust_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.insert_or_assign(long_key1, long_string1); 

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }

    SECTION("empty object with given allocator")
    {
        cust_json j1{alloc1};
        REQUIRE(j1.is_object());
        REQUIRE(alloc1 == j1.get_allocator());
    }

    SECTION("object move constructor")
    {
        cust_json j1{jsoncons::json_object_arg, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        j1.insert_or_assign(long_key1, long_string1); 

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator()); 
    }        

    /*SECTION("sorted policy iterator constructor")
    {
        std::map<std::string,double> m = {{"c",1},{"b",2},{"a",3}};

        cust_json j1{jsoncons::json_object_arg, m.begin(), m.end(), jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        REQUIRE(3 == j1.size());
        CHECK(3 == j1.at("a"));
        CHECK(2 == j1.at("b"));
        CHECK(1 == j1.at("c"));

        cust_json j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_json j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator());
    }
    SECTION("order preserving policy iterator constructor")
    {
        std::map<std::string,double> m = {{"c",1},{"b",2},{"a",3}};

        cust_ojson j1{jsoncons::json_object_arg, m.begin(), m.end(), jsoncons::semantic_tag::none, alloc1};
        REQUIRE(alloc1 == j1.get_allocator());
        REQUIRE(3 == j1.size());
        CHECK(3 == j1.at("a"));
        CHECK(2 == j1.at("b"));
        CHECK(1 == j1.at("c"));

        cust_ojson j2{std::move(j1)};
        REQUIRE(alloc1 == j2.get_allocator()); 

        cust_ojson j3{std::move(j2), alloc2};
        REQUIRE(alloc2 == j3.get_allocator());
    }*/
}

#endif // scoped_allocator

TEST_CASE("json constructor jsoncons::byte_string_arg tests")
{
    std::string expected_base64url = "Zm9vYmFy";

    SECTION("byte_string_arg std::vector<uint8_t>")
    {
        std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};
        jsoncons::json doc(jsoncons::byte_string_arg, bytes, jsoncons::semantic_tag::base64url);
        CHECK(doc.as<std::string>() == expected_base64url);
    }
    SECTION("byte_string_arg std::string")
    {
        std::string bytes = {'f','o','o','b','a','r'};
        jsoncons::json doc(jsoncons::byte_string_arg, bytes, jsoncons::semantic_tag::base64url);
        CHECK(doc.as<std::string>() == expected_base64url);
    }
}

TEST_CASE("json constructor tests")
{
    SECTION("json json_object_arg")
    {
        jsoncons::json j1(jsoncons::json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(1 == j1.size());
        CHECK(1 == j1.at("one").as<int>());

        jsoncons::json j2(jsoncons::json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(2 == j2.size());
        CHECK(1 == j2.at("one").as<int>());
        CHECK(2 == j2.at("two").as<int>());
    }
    SECTION("json json_array_arg")
    {
        jsoncons::json j1(jsoncons::json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(1 == j1.size());
        CHECK(1 == j1[0].as<int>());

        jsoncons::json j2(jsoncons::json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(2 == j2.size());
        CHECK(1 == j2[0].as<int>());
        CHECK(2 == j2[1].as<int>());
    }
    SECTION("ojson json_object_arg")
    {
        jsoncons::ojson j1(jsoncons::json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(1 == j1.size());
        CHECK(1 == j1.at("one").as<int>());

        jsoncons::ojson j2(jsoncons::json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(2 == j2.size());
        CHECK(1 == j2.at("one").as<int>());
        CHECK(2 == j2.at("two").as<int>());
    }
    SECTION("ojson json_array_arg")
    {
        jsoncons::ojson j1(jsoncons::json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(1 == j1.size());
        CHECK(1 == j1[0].as<int>());

        jsoncons::ojson j2(jsoncons::json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(2 == j2.size());
        CHECK(1 == j2[0].as<int>());
        CHECK(2 == j2[1].as<int>());
    }
    SECTION("json from key_value iterator")
    {
        using key_value_type = key_value<std::string, jsoncons::json>;
        std::vector<key_value_type> v = {key_value_type("string key too long for short string", "string value too long for short string"), key_value_type("and this one is also too long",2)};

        jsoncons::json j{jsoncons::json_object_arg, std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())};
        CHECK(j["string key too long for short string"].as_string_view() == jsoncons::string_view("string value too long for short string"));
        CHECK(v[0].value().is_null()); // moved
    }
    SECTION("ojson from key_value iterator")
    {
        using key_value_type = key_value<std::string, jsoncons::ojson>;
        std::vector<key_value_type> v = {key_value_type("string key too long for short string", "string value too long for short string"), key_value_type("and this one is also too long",2)};

        jsoncons::ojson j{jsoncons::json_object_arg, std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())};
        CHECK(j["string key too long for short string"].as_string_view() == jsoncons::string_view("string value too long for short string"));
        CHECK(v[0].value().is_null()); // moved
    }
    SECTION("json from std::pair iterator")
    {
        using key_value_type = std::pair<std::string, jsoncons::json>;
        std::vector<key_value_type> v = {key_value_type("string key too long for short string", "string value too long for short string"), key_value_type("and this one is also too long",2)};

        jsoncons::json j{jsoncons::json_object_arg, std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())};
        CHECK(j["string key too long for short string"].as_string_view() == jsoncons::string_view("string value too long for short string"));
        CHECK(v[0].second.is_null()); // moved
    }
    SECTION("ojson from std::pair iterator")
    {
        using key_value_type = std::pair<std::string, jsoncons::ojson>;
        std::vector<key_value_type> v = {key_value_type("string key too long for short string", "string value too long for short string"), key_value_type("and this one is also too long",2)};

        jsoncons::ojson j{jsoncons::json_object_arg, std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())};
        CHECK(j["string key too long for short string"].as_string_view() == jsoncons::string_view("string value too long for short string"));
        CHECK(v[0].second.is_null()); // moved
    }
}
TEST_CASE("json(string_view)")
{
    jsoncons::json::string_view_type sv("Hello world.");

    jsoncons::json doc(sv);

    CHECK(doc.as<json::string_view_type>() == sv);
    CHECK(doc.as_string_view() == sv);
}

TEST_CASE("json(string, jsoncons::semantic_tag::datetime)")
{
    std::string s("2015-05-07 12:41:07-07:00");

    jsoncons::json doc(s, jsoncons::semantic_tag::datetime);

    CHECK(doc.tag() == jsoncons::semantic_tag::datetime);
    CHECK(doc.as<std::string>() == s);
}


TEST_CASE("json(string, jsoncons::semantic_tag::epoch_second)")
{
    SECTION("positive integer")
    {
        int t = 10000;
        jsoncons::json doc(t, jsoncons::semantic_tag::epoch_second);

        CHECK(doc.tag() == jsoncons::semantic_tag::epoch_second);
        CHECK(doc.as<int>() == t);
    }
    SECTION("negative integer")
    {
        int t = -10000;
        jsoncons::json doc(t, jsoncons::semantic_tag::epoch_second);

        CHECK(doc.tag() == jsoncons::semantic_tag::epoch_second);
        CHECK(doc.as<int>() == t);
    }
    SECTION("floating point")
    {
        double t = 10000.1;
        jsoncons::json doc(t, jsoncons::semantic_tag::epoch_second);

        CHECK(doc.tag() == jsoncons::semantic_tag::epoch_second);
        CHECK(doc.as<double>() == t);
    }

}

TEST_CASE("json get_allocator() tests")
{
    SECTION("short string")
    {
        jsoncons::json doc("short");

        CHECK(doc.get_allocator() == jsoncons::json::allocator_type());
    }
    SECTION("long string")
    {
        jsoncons::json::allocator_type alloc;
        jsoncons::json doc("string too long for short string", alloc);

        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("byte string")
    {
        jsoncons::json::allocator_type alloc;
        jsoncons::json doc(jsoncons::byte_string({'H','e','l','l','o'}),alloc);

        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("array")
    {
        jsoncons::json::allocator_type alloc;
        jsoncons::json doc(jsoncons::json_array_arg, jsoncons::semantic_tag::none, alloc);

        REQUIRE(doc.is_array());
        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("object")
    {
        jsoncons::json::allocator_type alloc;
        jsoncons::json doc(jsoncons::json_object_arg, jsoncons::semantic_tag::none, alloc);

        REQUIRE(doc.is_object());
        CHECK(doc.get_allocator() == alloc);
    }
}

TEST_CASE("test_move_constructor")
{
    int64_t val1 = -100;
    jsoncons::json var1(val1, jsoncons::semantic_tag::none);
    jsoncons::json var2(std::move(var1));
    //CHECK(jsoncons::json_storage_kind::null == var1.storage_kind());
    CHECK(jsoncons::json_storage_kind::int64 == var2.storage_kind());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 9999;
    jsoncons::json var3(val3, jsoncons::semantic_tag::none);
    jsoncons::json var4(std::move(var3));
    //CHECK(jsoncons::json_storage_kind::null == var3.storage_kind());
    CHECK(jsoncons::json_storage_kind::uint64 == var4.storage_kind());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    jsoncons::json var5(val5, jsoncons::semantic_tag::none);
    jsoncons::json var6(std::move(var5));
    //CHECK(jsoncons::json_storage_kind::null == var5.storage_kind());
    CHECK(jsoncons::json_storage_kind::float64 == var6.storage_kind());
    CHECK(var6.as<double>() == val5);

    std::string val7("Too long for small string");
    jsoncons::json var7(val7.data(), val7.length(), jsoncons::semantic_tag::none);
    jsoncons::json var8(std::move(var7));
    //CHECK(jsoncons::json_storage_kind::null == var7.storage_kind());
    CHECK(jsoncons::json_storage_kind::long_str == var8.storage_kind());
    CHECK(val7 == var8.as<std::string>());

    std::string val9("Small string");
    jsoncons::json var9(val9.data(), val9.length(), jsoncons::semantic_tag::none);
    jsoncons::json var10(std::move(var9));
    //CHECK(jsoncons::json_storage_kind::null == var9.storage_kind());
    CHECK(jsoncons::json_storage_kind::short_str == var10.storage_kind());
    CHECK(val9 == var10.as<std::string>());

    bool val11 = true;
    jsoncons::json var11(val11, jsoncons::semantic_tag::none);
    jsoncons::json var12(std::move(var11));
    //CHECK(jsoncons::json_storage_kind::null == var11.storage_kind());
    CHECK(jsoncons::json_storage_kind::boolean == var12.storage_kind());
    CHECK(var12.as<bool>() == val11);

    std::string val13("Too long for small string");
    jsoncons::json var13(val13.data(), val13.length(), jsoncons::semantic_tag::none);
    jsoncons::json var14(std::move(var13));
    //CHECK(jsoncons::json_storage_kind::null == var13.storage_kind());
    CHECK(jsoncons::json_storage_kind::long_str == var14.storage_kind());
    CHECK(val13 == var14.as<std::string>());

    jsoncons::json val15(jsoncons::json_object_arg, {{"first",1},{"second",2} });
    jsoncons::json var15(val15);
    jsoncons::json var16(std::move(var15));
    CHECK(jsoncons::json_storage_kind::null == var15.storage_kind());
    CHECK(jsoncons::json_storage_kind::object == var16.storage_kind());
    CHECK(val15 == var16);

    jsoncons::json::array val17 = {1,2,3,4};
    jsoncons::json var17(val17, jsoncons::semantic_tag::none);
    jsoncons::json var18(std::move(var17));
    CHECK(jsoncons::json_storage_kind::null == var17.storage_kind());
    CHECK(jsoncons::json_storage_kind::array == var18.storage_kind());
    CHECK(val17 == var18);
}

TEST_CASE("test_copy_constructor")
{
    int64_t val1 = 123456789;
    jsoncons::json var1(val1, jsoncons::semantic_tag::none);
    jsoncons::json var2(var1);
    CHECK(jsoncons::json_storage_kind::int64 == var1.storage_kind());
    CHECK(jsoncons::json_storage_kind::int64 == var2.storage_kind());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 123456789;
    jsoncons::json var3(val3, jsoncons::semantic_tag::none);
    jsoncons::json var4(var3);
    CHECK(jsoncons::json_storage_kind::uint64 == var3.storage_kind());
    CHECK(jsoncons::json_storage_kind::uint64 == var4.storage_kind());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    jsoncons::json var5(val5, jsoncons::semantic_tag::none);
    jsoncons::json var6(var5);
    CHECK(jsoncons::json_storage_kind::float64 == var5.storage_kind());
    CHECK(jsoncons::json_storage_kind::float64 == var6.storage_kind());
    CHECK(var6.as<double>() == val5);

    std::string val9 = "Small string";
    jsoncons::json var9(val9.data(), val9.length(), jsoncons::semantic_tag::none);
    jsoncons::json var10(var9);
    CHECK(jsoncons::json_storage_kind::short_str == var9.storage_kind());
    CHECK(jsoncons::json_storage_kind::short_str == var10.storage_kind());
    CHECK(var10.as<std::string>() == val9);

    bool val11 = true;
    jsoncons::json var11(val11, jsoncons::semantic_tag::none);
    jsoncons::json var12(var11);
    CHECK(jsoncons::json_storage_kind::boolean == var11.storage_kind());
    CHECK(jsoncons::json_storage_kind::boolean == var12.storage_kind());
    CHECK(var12.as<bool>() == val11);

    std::string val13 = "Too long for small string";
    jsoncons::json var13(val13.data(), val13.length(), jsoncons::semantic_tag::none);
    jsoncons::json var14(var13);
    CHECK(jsoncons::json_storage_kind::long_str == var13.storage_kind());
    CHECK(jsoncons::json_storage_kind::long_str == var14.storage_kind());
    CHECK(var14.as<std::string>() == val13);

    jsoncons::json val15(jsoncons::json_object_arg, { {"first",1},{"second",2} });
    jsoncons::json var15(val15);
    jsoncons::json var16(var15);
    CHECK(jsoncons::json_storage_kind::object == var15.storage_kind());
    CHECK(jsoncons::json_storage_kind::object == var16.storage_kind());
    CHECK(val15 == var16);

    jsoncons::json val17(jsoncons::json_array_arg, {1,2,3,4});
    jsoncons::json var17(val17);
    jsoncons::json var18(var17);
    CHECK(jsoncons::json_storage_kind::array == var17.storage_kind());
    CHECK(jsoncons::json_storage_kind::array == var18.storage_kind());
    CHECK(val17 == var18);
}

#if (defined(__GNUC__) || defined(__clang__)) && defined(JSONCONS_HAS_INT128) 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
TEST_CASE("json constructor __int64 tests")
{
    SECTION("test 1")
    {
        jsoncons::json j1("-18446744073709551617", jsoncons::semantic_tag::bigint);

        __int128 val1 = j1.as<__int128>();

        jsoncons::json j2(val1);
        CHECK((j2 == j1));

        __int128 val2 = j2.as<__int128>();
        CHECK((val2 == val1));
    }
}
TEST_CASE("json constructor unsigned __int64 tests")
{
    SECTION("test 1")
    {
        jsoncons::json j1("18446744073709551616", jsoncons::semantic_tag::bigint);

        __int128 val1 = j1.as<__int128>();

        jsoncons::json j2(val1);
        CHECK((j2 == j1));

        __int128 val2 = j2.as<__int128>();
        CHECK((val2 == val1));
    }
}
#pragma GCC diagnostic pop
#endif

