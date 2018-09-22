// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <new>
#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

TEST_CASE("test_variant")
{
    json::variant var1(int64_t(-100));
    CHECK(data_type_tag::int64_tag == var1.data_type());
    json::variant var2(uint64_t(100));
    CHECK(data_type_tag::uint64_tag == var2.data_type());
    json::variant var3("Small string",12);
    CHECK(data_type_tag::short_string_tag == var3.data_type());
    json::variant var4("Too long to fit in small string",31);
    CHECK(data_type_tag::long_string_tag == var4.data_type());
    json::variant var5(true);
    CHECK(data_type_tag::bool_tag == var5.data_type());
    json::variant var6;
    CHECK(data_type_tag::empty_object_tag == var6.data_type());
    json::variant var7{ null_type() };
    CHECK(data_type_tag::null_tag == var7.data_type());
    json::variant var8{ json::allocator_type() };
    CHECK(data_type_tag::object_tag == var8.data_type());
    json::variant var9(123456789.9);
    CHECK(data_type_tag::double_tag == var9.data_type());
}

TEST_CASE("test_move_constructor")
{
    int64_t val1 = -100;
    json::variant var1(val1);
    json::variant var2(std::move(var1));
    //CHECK(data_type_tag::null_tag == var1.data_type());
    CHECK(data_type_tag::int64_tag == var2.data_type());
    CHECK(var2.int64_data_cast()->value() == val1);

    uint64_t val3 = 9999;
    json::variant var3(val3);
    json::variant var4(std::move(var3));
    //CHECK(data_type_tag::null_tag == var3.data_type());
    CHECK(data_type_tag::uint64_tag == var4.data_type());
    CHECK(var4.uint64_data_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5);
    json::variant var6(std::move(var5));
    //CHECK(data_type_tag::null_tag == var5.data_type());
    CHECK(data_type_tag::double_tag == var6.data_type());
    CHECK(var6.double_data_cast()->value() == val5);

    std::string val7("Too long for small string");
    json::variant var7(val7.data(),val7.length());
    json::variant var8(std::move(var7));
    //CHECK(data_type_tag::null_tag == var7.data_type());
    CHECK(data_type_tag::long_string_tag == var8.data_type());
    CHECK(val7 == var8.string_data_cast()->data());
    CHECK(val7.length() == var8.string_data_cast()->length());

    std::string val9("Small string");
    json::variant var9(val9.data(), val9.length());
    json::variant var10(std::move(var9));
    //CHECK(data_type_tag::null_tag == var9.data_type());
    CHECK(data_type_tag::short_string_tag == var10.data_type());
    CHECK(val9 == var10.short_string_data_cast()->data());
    CHECK(val9.length() == var10.short_string_data_cast()->length());

    bool val11 = true;
    json::variant var11(val11);
    json::variant var12(std::move(var11));
    //CHECK(data_type_tag::null_tag == var11.data_type());
    CHECK(data_type_tag::bool_tag == var12.data_type());
    CHECK(var12.bool_data_cast()->value() == val11);

    std::string val13("Too long for small string");
    json::variant var13(val13.data(), val13.length());
    json::variant var14(std::move(var13));
    //CHECK(data_type_tag::null_tag == var13.data_type());
    CHECK(data_type_tag::long_string_tag == var14.data_type());
    CHECK(val13 == var14.string_data_cast()->data());

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15);
    json::variant var16(std::move(var15));
    CHECK(data_type_tag::null_tag == var15.data_type());
    CHECK(data_type_tag::object_tag == var16.data_type());
    CHECK(val15 == var16.object_data_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17);
    json::variant var18(std::move(var17));
    CHECK(data_type_tag::null_tag == var17.data_type());
    CHECK(data_type_tag::array_tag == var18.data_type());
    CHECK(val17 == var18.array_data_cast()->value());
}

TEST_CASE("test_copy_constructor")
{
    int64_t val1 = 123456789;
    json::variant var1(val1);
    json::variant var2(var1);
    CHECK(data_type_tag::int64_tag == var1.data_type());
    CHECK(data_type_tag::int64_tag == var2.data_type());
    CHECK(var2.int64_data_cast()->value() == val1);

    uint64_t val3 = 123456789;
    json::variant var3(val3);
    json::variant var4(var3);
    CHECK(data_type_tag::uint64_tag == var3.data_type());
    CHECK(data_type_tag::uint64_tag == var4.data_type());
    CHECK(var4.uint64_data_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5,floating_point_options());
    json::variant var6(var5);
    CHECK(data_type_tag::double_tag == var5.data_type());
    CHECK(data_type_tag::double_tag == var6.data_type());
    CHECK(var6.double_data_cast()->value() == val5);

    std::string val9 = "Small string";
    json::variant var9(val9.data(),val9.length());
    json::variant var10(var9);
    CHECK(data_type_tag::short_string_tag == var9.data_type());
    CHECK(data_type_tag::short_string_tag == var10.data_type());
    CHECK(var10.short_string_data_cast()->data() == val9);

    bool val11 = true;
    json::variant var11(val11);
    json::variant var12(var11);
    CHECK(data_type_tag::bool_tag == var11.data_type());
    CHECK(data_type_tag::bool_tag == var12.data_type());
    CHECK(var12.bool_data_cast()->value() == val11);

    std::string val13 = "Too long for small string";
    json::variant var13(val13.data(),val13.length());
    json::variant var14(var13);
    CHECK(data_type_tag::long_string_tag == var13.data_type());
    CHECK(data_type_tag::long_string_tag == var14.data_type());
    CHECK(var14.string_data_cast()->data() == val13);

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15);
    json::variant var16(var15);
    CHECK(data_type_tag::object_tag == var15.data_type());
    CHECK(data_type_tag::object_tag == var16.data_type());
    CHECK(val15 == var16.object_data_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17);
    json::variant var18(var17);
    CHECK(data_type_tag::array_tag == var17.data_type());
    CHECK(data_type_tag::array_tag == var18.data_type());
    CHECK(val17 == var18.array_data_cast()->value());
}

TEST_CASE("test_equals")
{
    json::variant var1;
    json::variant var2{ json::object() };
    CHECK((var1 == var1 && var2 == var2));
    CHECK((var1 == var2 && var2 == var1));

    json::variant var3;
    CHECK((var3 == var1 && var1 == var3));
    json::variant var4{ json::object({{"first",1},{"second",2}}) };
    json::variant var5{ json::object({ { "first",1 },{ "second",2 } }) };
    CHECK((var3 != var4 && var4 != var3));
    CHECK((var2 != var4 && var4 != var2));
    CHECK(var4 == var4);
    CHECK(var4 == var5);
    CHECK(var5 == var4);

    json::variant var6(int64_t(100));
    json::variant var7(uint64_t(100));
    CHECK((var6 == var7 && var7 == var6));

    json::variant var8(100.0);
    CHECK((var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7));

    std::string val9("small string");
    std::string val11("small string 2");
    json::variant var9(val9.data(),val9.length());
    json::variant var10(val9.data(),val9.length());
    json::variant var11(val11.data(),val11.length());

    std::string val12("too long for small string");
    std::string val14("too long for small string 2");
    json::variant var12(val12.data(),val12.length());
    json::variant var13(val12.data(),val12.length());
    json::variant var14(val14.data(),val14.length());
    CHECK((var9 == var10 && var10 == var9));
    CHECK((var9 != var11 && var11 != var9));
    CHECK((var12 == var13 && var13 == var12));
    CHECK((var12 != var14 && var14 != var12));

    json::variant var15(val9.data(),val9.length(),std::allocator<char>());
    CHECK((var9 == var15 && var15 == var9));

    json::variant var16(static_cast<int64_t>(0));
    json::variant var17(static_cast<uint64_t>(0));
    CHECK(var16 == var17);
    CHECK(var17 == var16);
}

