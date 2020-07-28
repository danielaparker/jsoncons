// Copyright 2018 Daniel Parker
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

TEST_CASE("json constructor byte_string_arg tests")
{
    std::string expected_base64url = "Zm9vYmFy";

    SECTION("byte_string_arg std::vector<uint8_t>")
    {
        std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};
        json j(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(j.as<std::string>() == expected_base64url);
    }
    SECTION("byte_string_arg std::string")
    {
        std::string bytes = {'f','o','o','b','a','r'};
        json j(byte_string_arg, bytes, semantic_tag::base64url);
        CHECK(j.as<std::string>() == expected_base64url);
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

    json j(sv);

    CHECK(j.as<json::string_view_type>() == sv);
    CHECK(j.as_string_view() == sv);
}

TEST_CASE("json(string, semantic_tag::datetime)")
{
    std::string s("2015-05-07 12:41:07-07:00");

    json j(s, semantic_tag::datetime);

    CHECK(j.tag() == semantic_tag::datetime);
    CHECK(j.as<std::string>() == s);
}


TEST_CASE("json(string, semantic_tag::epoch_second)")
{
    SECTION("positive integer")
    {
        int t = 10000;
        json j(t, semantic_tag::epoch_second);

        CHECK(j.tag() == semantic_tag::epoch_second);
        CHECK(j.as<int>() == t);
    }
    SECTION("negative integer")
    {
        int t = -10000;
        json j(t, semantic_tag::epoch_second);

        CHECK(j.tag() == semantic_tag::epoch_second);
        CHECK(j.as<int>() == t);
    }
    SECTION("floating point")
    {
        double t = 10000.1;
        json j(t, semantic_tag::epoch_second);

        CHECK(j.tag() == semantic_tag::epoch_second);
        CHECK(j.as<double>() == t);
    }

}

TEST_CASE("json get_allocator() tests")
{
    SECTION("short string")
    {
        json j("short");

        CHECK(j.get_allocator() == json::allocator_type());
    }
    SECTION("long string")
    {
        json::allocator_type alloc;
        json j("string too long for short string", alloc);

        CHECK(j.get_allocator() == alloc);
    }
    SECTION("byte string")
    {
        json::allocator_type alloc;
        json j(byte_string({'H','e','l','l','o'}),alloc);

        CHECK(j.get_allocator() == alloc);
    }
    SECTION("array")
    {
        json::allocator_type alloc;
        json j(json_array_arg, semantic_tag::none, alloc);

        REQUIRE(j.is_array());
        CHECK(j.get_allocator() == alloc);
    }
    SECTION("object")
    {
        json::allocator_type alloc;
        json j(json_object_arg, semantic_tag::none, alloc);

        REQUIRE(j.is_object());
        CHECK(j.get_allocator() == alloc);
    }
}

TEST_CASE("test_move_constructor")
{
    int64_t val1 = -100;
    json var1(val1, semantic_tag::none);
    json var2(std::move(var1));
    //CHECK(storage_kind::null_value == var1.storage());
    CHECK(storage_kind::int64_value == var2.storage());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 9999;
    json var3(val3, semantic_tag::none);
    json var4(std::move(var3));
    //CHECK(storage_kind::null_value == var3.storage());
    CHECK(storage_kind::uint64_value == var4.storage());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    json var5(val5, semantic_tag::none);
    json var6(std::move(var5));
    //CHECK(storage_kind::null_value == var5.storage());
    CHECK(storage_kind::double_value == var6.storage());
    CHECK(var6.as<double>() == val5);

    std::string val7("Too long for small string");
    json var7(val7.data(), val7.length(), semantic_tag::none);
    json var8(std::move(var7));
    //CHECK(storage_kind::null_value == var7.storage());
    CHECK(storage_kind::long_string_value == var8.storage());
    CHECK(val7 == var8.as<std::string>());

    std::string val9("Small string");
    json var9(val9.data(), val9.length(), semantic_tag::none);
    json var10(std::move(var9));
    //CHECK(storage_kind::null_value == var9.storage());
    CHECK(storage_kind::short_string_value == var10.storage());
    CHECK(val9 == var10.as<std::string>());

    bool val11 = true;
    json var11(val11, semantic_tag::none);
    json var12(std::move(var11));
    //CHECK(storage_kind::null_value == var11.storage());
    CHECK(storage_kind::bool_value == var12.storage());
    CHECK(var12.as<bool>() == val11);

    std::string val13("Too long for small string");
    json var13(val13.data(), val13.length(), semantic_tag::none);
    json var14(std::move(var13));
    //CHECK(storage_kind::null_value == var13.storage());
    CHECK(storage_kind::long_string_value == var14.storage());
    CHECK(val13 == var14.as<std::string>());

    json::object val15 = { {"first",1},{"second",2} };
    json var15(val15, semantic_tag::none);
    json var16(std::move(var15));
    CHECK(storage_kind::null_value == var15.storage());
    CHECK(storage_kind::object_value == var16.storage());
    CHECK(val15 == var16);

    json::array val17 = {1,2,3,4};
    json var17(val17, semantic_tag::none);
    json var18(std::move(var17));
    CHECK(storage_kind::null_value == var17.storage());
    CHECK(storage_kind::array_value == var18.storage());
    CHECK(val17 == var18);
}

TEST_CASE("test_copy_constructor")
{
    int64_t val1 = 123456789;
    json var1(val1, semantic_tag::none);
    json var2(var1);
    CHECK(storage_kind::int64_value == var1.storage());
    CHECK(storage_kind::int64_value == var2.storage());
    CHECK(var2.as<int64_t>() == val1);

    uint64_t val3 = 123456789;
    json var3(val3, semantic_tag::none);
    json var4(var3);
    CHECK(storage_kind::uint64_value == var3.storage());
    CHECK(storage_kind::uint64_value == var4.storage());
    CHECK(var4.as<uint64_t>() == val3);

    double val5 = 123456789.9;
    json var5(val5, semantic_tag::none);
    json var6(var5);
    CHECK(storage_kind::double_value == var5.storage());
    CHECK(storage_kind::double_value == var6.storage());
    CHECK(var6.as<double>() == val5);

    std::string val9 = "Small string";
    json var9(val9.data(), val9.length(), semantic_tag::none);
    json var10(var9);
    CHECK(storage_kind::short_string_value == var9.storage());
    CHECK(storage_kind::short_string_value == var10.storage());
    CHECK(var10.as<std::string>() == val9);

    bool val11 = true;
    json var11(val11, semantic_tag::none);
    json var12(var11);
    CHECK(storage_kind::bool_value == var11.storage());
    CHECK(storage_kind::bool_value == var12.storage());
    CHECK(var12.as<bool>() == val11);

    std::string val13 = "Too long for small string";
    json var13(val13.data(), val13.length(), semantic_tag::none);
    json var14(var13);
    CHECK(storage_kind::long_string_value == var13.storage());
    CHECK(storage_kind::long_string_value == var14.storage());
    CHECK(var14.as<std::string>() == val13);

    json::object val15 = { {"first",1},{"second",2} };
    json var15(val15, semantic_tag::none);
    json var16(var15);
    CHECK(storage_kind::object_value == var15.storage());
    CHECK(storage_kind::object_value == var16.storage());
    CHECK(val15 == var16);

    json::array val17 = {1,2,3,4};
    json var17(val17, semantic_tag::none);
    json var18(var17);
    CHECK(storage_kind::array_value == var17.storage());
    CHECK(storage_kind::array_value == var18.storage());
    CHECK(val17 == var18);
}

