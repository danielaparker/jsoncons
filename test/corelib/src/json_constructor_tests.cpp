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


TEST_CASE("json constructor with alloc arg")
{
    SECTION("test 1")
    {
        char buffer1[1024] = {}; // a small buffer on the stack
        std::pmr::monotonic_buffer_resource pool1{ std::data(buffer1), std::size(buffer1) };
        std::pmr::polymorphic_allocator<char> alloc1(&pool1);
        pmr_json j1{ alloc1 };

        const char* long_key1 = "Key too long for short string";
        const char* long_string1 = "String too long for short string";
        j1[long_key1] = long_string1;

        char* last1 = buffer1 + sizeof(buffer1);
        auto it = std::search(buffer1, last1, long_string1, long_string1 + strlen(long_string1));
        CHECK(it != last1);

        it = std::search(buffer1, last1, long_key1, long_key1 + strlen(long_key1));
        CHECK(it != last1);
    }
    SECTION("test 2")
    {
        const char* long_string1 = "String too long for short string";
        pmr_json j1(long_string1);
        
        char buffer2[1024] = {}; // a small buffer on the stack
        std::pmr::monotonic_buffer_resource pool2{ std::data(buffer2), std::size(buffer2) };
        std::pmr::polymorphic_allocator<char> alloc2(&pool2);
        pmr_json j2{j1, alloc2};
        
        char* last2 = buffer2 + sizeof(buffer2);
        auto it = std::search(buffer2, last2, long_string1, long_string1+strlen(long_string1));
        CHECK(it != last2);
    }
}

#endif

TEST_CASE("json constructor byte_string_arg tests")
{
    std::string expected_base64url = "Zm9vYmFy";

    SECTION("byte_string_arg std::vector<uint8_t>")
    {
        std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};
        json doc(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(doc.as<std::string>() == expected_base64url);
    }
    SECTION("byte_string_arg std::string")
    {
        std::string bytes = {'f','o','o','b','a','r'};
        json doc(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(doc.as<std::string>() == expected_base64url);
    }
}

TEST_CASE("json constructor tests")
{
    SECTION("json json_object_arg")
    {
        json j1(json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(j1.size() == 1);
        CHECK(j1.at("one").as<int>() == 1);

        json j2(json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(j2.size() == 2);
        CHECK(j2.at("one").as<int>() == 1);
        CHECK(j2.at("two").as<int>() == 2);
    }
    SECTION("json json_array_arg")
    {
        json j1(json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(j1.size() == 1);
        CHECK(j1[0].as<int>() == 1);

        json j2(json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(j2.size() == 2);
        CHECK(j2[0].as<int>() == 1);
        CHECK(j2[1].as<int>() == 2);
    }
    SECTION("ojson json_object_arg")
    {
        ojson j1(json_object_arg, {{"one",1}});
        REQUIRE(j1.is_object());
        REQUIRE(j1.size() == 1);
        CHECK(j1.at("one").as<int>() == 1);

        ojson j2(json_object_arg, {{"one",1},{"two",2}});
        REQUIRE(j2.is_object());
        REQUIRE(j2.size() == 2);
        CHECK(j2.at("one").as<int>() == 1);
        CHECK(j2.at("two").as<int>() == 2);
    }
    SECTION("ojson json_array_arg")
    {
        ojson j1(json_array_arg, {1});
        REQUIRE(j1.is_array());
        REQUIRE(j1.size() == 1);
        CHECK(j1[0].as<int>() == 1);

        ojson j2(json_array_arg, {1,2});
        REQUIRE(j2.is_array());
        REQUIRE(j2.size() == 2);
        CHECK(j2[0].as<int>() == 1);
        CHECK(j2[1].as<int>() == 2);
    }
}
TEST_CASE("json(string_view)")
{
    json::string_view_type sv("Hello world.");

    json doc(sv);

    CHECK(doc.as<json::string_view_type>() == sv);
    CHECK(doc.as_string_view() == sv);
}

TEST_CASE("json(string, semantic_tag::datetime)")
{
    std::string s("2015-05-07 12:41:07-07:00");

    json doc(s, semantic_tag::datetime);

    CHECK(doc.tag() == semantic_tag::datetime);
    CHECK(doc.as<std::string>() == s);
}


TEST_CASE("json(string, semantic_tag::epoch_second)")
{
    SECTION("positive integer")
    {
        int t = 10000;
        json doc(t, semantic_tag::epoch_second);

        CHECK(doc.tag() == semantic_tag::epoch_second);
        CHECK(doc.as<int>() == t);
    }
    SECTION("negative integer")
    {
        int t = -10000;
        json doc(t, semantic_tag::epoch_second);

        CHECK(doc.tag() == semantic_tag::epoch_second);
        CHECK(doc.as<int>() == t);
    }
    SECTION("floating point")
    {
        double t = 10000.1;
        json doc(t, semantic_tag::epoch_second);

        CHECK(doc.tag() == semantic_tag::epoch_second);
        CHECK(doc.as<double>() == t);
    }

}

TEST_CASE("json get_allocator() tests")
{
    SECTION("short string")
    {
        json doc("short");

        CHECK(doc.get_allocator() == json::allocator_type());
    }
    SECTION("long string")
    {
        json::allocator_type alloc;
        json doc("string too long for short string", alloc);

        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("byte string")
    {
        json::allocator_type alloc;
        json doc(byte_string({'H','e','l','l','o'}),alloc);

        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("array")
    {
        json::allocator_type alloc;
        json doc(json_array_arg, semantic_tag::none, alloc);

        REQUIRE(doc.is_array());
        CHECK(doc.get_allocator() == alloc);
    }
    SECTION("object")
    {
        json::allocator_type alloc;
        json doc(json_object_arg, semantic_tag::none, alloc);

        REQUIRE(doc.is_object());
        CHECK(doc.get_allocator() == alloc);
    }
}

TEST_CASE("test_move_constructor")
{
    int64_t val1 = -100;
    json var1(val1, semantic_tag::none);
    json var2(std::move(var1));
    //CHECK(json_storage_kind::null == var1.storage_kind());
    CHECK(json_storage_kind::int64 == var2.storage_kind());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 9999;
    json var3(val3, semantic_tag::none);
    json var4(std::move(var3));
    //CHECK(json_storage_kind::null == var3.storage_kind());
    CHECK(json_storage_kind::uint64 == var4.storage_kind());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    json var5(val5, semantic_tag::none);
    json var6(std::move(var5));
    //CHECK(json_storage_kind::null == var5.storage_kind());
    CHECK(json_storage_kind::float64 == var6.storage_kind());
    CHECK(var6.as<double>() == val5);

    std::string val7("Too long for small string");
    json var7(val7.data(), val7.length(), semantic_tag::none);
    json var8(std::move(var7));
    //CHECK(json_storage_kind::null == var7.storage_kind());
    CHECK(json_storage_kind::long_str == var8.storage_kind());
    CHECK(val7 == var8.as<std::string>());

    std::string val9("Small string");
    json var9(val9.data(), val9.length(), semantic_tag::none);
    json var10(std::move(var9));
    //CHECK(json_storage_kind::null == var9.storage_kind());
    CHECK(json_storage_kind::short_str == var10.storage_kind());
    CHECK(val9 == var10.as<std::string>());

    bool val11 = true;
    json var11(val11, semantic_tag::none);
    json var12(std::move(var11));
    //CHECK(json_storage_kind::null == var11.storage_kind());
    CHECK(json_storage_kind::boolean == var12.storage_kind());
    CHECK(var12.as<bool>() == val11);

    std::string val13("Too long for small string");
    json var13(val13.data(), val13.length(), semantic_tag::none);
    json var14(std::move(var13));
    //CHECK(json_storage_kind::null == var13.storage_kind());
    CHECK(json_storage_kind::long_str == var14.storage_kind());
    CHECK(val13 == var14.as<std::string>());

    json val15(json_object_arg, {{"first",1},{"second",2} });
    json var15(val15);
    json var16(std::move(var15));
    CHECK(json_storage_kind::null == var15.storage_kind());
    CHECK(json_storage_kind::object == var16.storage_kind());
    CHECK(val15 == var16);

    json::array val17 = {1,2,3,4};
    json var17(val17, semantic_tag::none);
    json var18(std::move(var17));
    CHECK(json_storage_kind::null == var17.storage_kind());
    CHECK(json_storage_kind::array == var18.storage_kind());
    CHECK(val17 == var18);
}

TEST_CASE("test_copy_constructor")
{
    int64_t val1 = 123456789;
    json var1(val1, semantic_tag::none);
    json var2(var1);
    CHECK(json_storage_kind::int64 == var1.storage_kind());
    CHECK(json_storage_kind::int64 == var2.storage_kind());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 123456789;
    json var3(val3, semantic_tag::none);
    json var4(var3);
    CHECK(json_storage_kind::uint64 == var3.storage_kind());
    CHECK(json_storage_kind::uint64 == var4.storage_kind());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    json var5(val5, semantic_tag::none);
    json var6(var5);
    CHECK(json_storage_kind::float64 == var5.storage_kind());
    CHECK(json_storage_kind::float64 == var6.storage_kind());
    CHECK(var6.as<double>() == val5);

    std::string val9 = "Small string";
    json var9(val9.data(), val9.length(), semantic_tag::none);
    json var10(var9);
    CHECK(json_storage_kind::short_str == var9.storage_kind());
    CHECK(json_storage_kind::short_str == var10.storage_kind());
    CHECK(var10.as<std::string>() == val9);

    bool val11 = true;
    json var11(val11, semantic_tag::none);
    json var12(var11);
    CHECK(json_storage_kind::boolean == var11.storage_kind());
    CHECK(json_storage_kind::boolean == var12.storage_kind());
    CHECK(var12.as<bool>() == val11);

    std::string val13 = "Too long for small string";
    json var13(val13.data(), val13.length(), semantic_tag::none);
    json var14(var13);
    CHECK(json_storage_kind::long_str == var13.storage_kind());
    CHECK(json_storage_kind::long_str == var14.storage_kind());
    CHECK(var14.as<std::string>() == val13);

    json val15(json_object_arg, { {"first",1},{"second",2} });
    json var15(val15);
    json var16(var15);
    CHECK(json_storage_kind::object == var15.storage_kind());
    CHECK(json_storage_kind::object == var16.storage_kind());
    CHECK(val15 == var16);

    json val17(json_array_arg, {1,2,3,4});
    json var17(val17);
    json var18(var17);
    CHECK(json_storage_kind::array == var17.storage_kind());
    CHECK(json_storage_kind::array == var18.storage_kind());
    CHECK(val17 == var18);
}

#if (defined(__GNUC__) || defined(__clang__)) && defined(JSONCONS_HAS_INT128) 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
TEST_CASE("json constructor __int64 tests")
{
    SECTION("test 1")
    {
        json j1("-18446744073709551617", semantic_tag::bigint);

        __int128 val1 = j1.as<__int128>();

        json j2(val1);
        CHECK((j2 == j1));

        __int128 val2 = j2.as<__int128>();
        CHECK((val2 == val1));
    }
}
TEST_CASE("json constructor unsigned __int64 tests")
{
    SECTION("test 1")
    {
        json j1("18446744073709551616", semantic_tag::bigint);

        __int128 val1 = j1.as<__int128>();

        json j2(val1);
        CHECK((j2 == j1));

        __int128 val2 = j2.as<__int128>();
        CHECK((val2 == val1));
    }
}
#pragma GCC diagnostic pop
#endif
