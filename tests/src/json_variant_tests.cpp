// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_exception.hpp>
#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <new>
#include <string>

using namespace jsoncons;

TEST_CASE("test_variant")
{
    json::variant var1(int64_t(-100), semantic_tag::none);
    CHECK(value_kind::int64_value == var1.kind());
    json::variant var2(uint64_t(100), semantic_tag::none);
    CHECK(value_kind::uint64_value == var2.kind());
    json::variant var3("Small string", 12, semantic_tag::none);
    CHECK(value_kind::short_string_value == var3.kind());
    json::variant var4("Too long to fit in small string", 31, semantic_tag::none);
    CHECK(value_kind::long_string_value == var4.kind());
    json::variant var5(true, semantic_tag::none);
    CHECK(value_kind::bool_value == var5.kind());
    json::variant var6(semantic_tag::none);
    CHECK(value_kind::empty_object_value == var6.kind());
    json::variant var7{ null_type(), semantic_tag::none };
    CHECK(value_kind::null_value == var7.kind());
    json::variant var8{ json::object(json::allocator_type()), semantic_tag::none };
    CHECK(value_kind::object_value == var8.kind());
    json::variant var9(123456789.9, semantic_tag::none);
    CHECK(value_kind::double_value == var9.kind());
}

TEST_CASE("test_move_constructor")
{
    int64_t val1 = -100;
    json::variant var1(val1, semantic_tag::none);
    json::variant var2(std::move(var1));
    //CHECK(value_kind::null_value == var1.kind());
    CHECK(value_kind::int64_value == var2.kind());
    CHECK(var2.int64_holder_cast()->value() == val1);

    uint64_t val3 = 9999;
    json::variant var3(val3, semantic_tag::none);
    json::variant var4(std::move(var3));
    //CHECK(value_kind::null_value == var3.kind());
    CHECK(value_kind::uint64_value == var4.kind());
    CHECK(var4.uint64_holder_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5, semantic_tag::none);
    json::variant var6(std::move(var5));
    //CHECK(value_kind::null_value == var5.kind());
    CHECK(value_kind::double_value == var6.kind());
    CHECK(var6.double_holder_cast()->value() == val5);

    std::string val7("Too long for small string");
    json::variant var7(val7.data(), val7.length(), semantic_tag::none);
    json::variant var8(std::move(var7));
    //CHECK(value_kind::null_value == var7.kind());
    CHECK(value_kind::long_string_value == var8.kind());
    CHECK(val7 == var8.string_holder_cast()->data());
    CHECK(val7.length() == var8.string_holder_cast()->length());

    std::string val9("Small string");
    json::variant var9(val9.data(), val9.length(), semantic_tag::none);
    json::variant var10(std::move(var9));
    //CHECK(value_kind::null_value == var9.kind());
    CHECK(value_kind::short_string_value == var10.kind());
    CHECK(val9 == var10.short_string_holder_cast()->data());
    CHECK(val9.length() == var10.short_string_holder_cast()->length());

    bool val11 = true;
    json::variant var11(val11, semantic_tag::none);
    json::variant var12(std::move(var11));
    //CHECK(value_kind::null_value == var11.kind());
    CHECK(value_kind::bool_value == var12.kind());
    CHECK(var12.bool_holder_cast()->value() == val11);

    std::string val13("Too long for small string");
    json::variant var13(val13.data(), val13.length(), semantic_tag::none);
    json::variant var14(std::move(var13));
    //CHECK(value_kind::null_value == var13.kind());
    CHECK(value_kind::long_string_value == var14.kind());
    CHECK(val13 == var14.string_holder_cast()->data());

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15, semantic_tag::none);
    json::variant var16(std::move(var15));
    CHECK(value_kind::null_value == var15.kind());
    CHECK(value_kind::object_value == var16.kind());
    CHECK(val15 == var16.object_holder_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17, semantic_tag::none);
    json::variant var18(std::move(var17));
    CHECK(value_kind::null_value == var17.kind());
    CHECK(value_kind::array_value == var18.kind());
    CHECK(val17 == var18.array_holder_cast()->value());
}

TEST_CASE("test_copy_constructor")
{
    int64_t val1 = 123456789;
    json::variant var1(val1, semantic_tag::none);
    json::variant var2(var1);
    CHECK(value_kind::int64_value == var1.kind());
    CHECK(value_kind::int64_value == var2.kind());
    CHECK(var2.int64_holder_cast()->value() == val1);

    uint64_t val3 = 123456789;
    json::variant var3(val3, semantic_tag::none);
    json::variant var4(var3);
    CHECK(value_kind::uint64_value == var3.kind());
    CHECK(value_kind::uint64_value == var4.kind());
    CHECK(var4.uint64_holder_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5, semantic_tag::none);
    json::variant var6(var5);
    CHECK(value_kind::double_value == var5.kind());
    CHECK(value_kind::double_value == var6.kind());
    CHECK(var6.double_holder_cast()->value() == val5);

    std::string val9 = "Small string";
    json::variant var9(val9.data(), val9.length(), semantic_tag::none);
    json::variant var10(var9);
    CHECK(value_kind::short_string_value == var9.kind());
    CHECK(value_kind::short_string_value == var10.kind());
    CHECK(var10.short_string_holder_cast()->data() == val9);

    bool val11 = true;
    json::variant var11(val11, semantic_tag::none);
    json::variant var12(var11);
    CHECK(value_kind::bool_value == var11.kind());
    CHECK(value_kind::bool_value == var12.kind());
    CHECK(var12.bool_holder_cast()->value() == val11);

    std::string val13 = "Too long for small string";
    json::variant var13(val13.data(), val13.length(), semantic_tag::none);
    json::variant var14(var13);
    CHECK(value_kind::long_string_value == var13.kind());
    CHECK(value_kind::long_string_value == var14.kind());
    CHECK(var14.string_holder_cast()->data() == val13);

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15, semantic_tag::none);
    json::variant var16(var15);
    CHECK(value_kind::object_value == var15.kind());
    CHECK(value_kind::object_value == var16.kind());
    CHECK(val15 == var16.object_holder_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17, semantic_tag::none);
    json::variant var18(var17);
    CHECK(value_kind::array_value == var17.kind());
    CHECK(value_kind::array_value == var18.kind());
    CHECK(val17 == var18.array_holder_cast()->value());
}

TEST_CASE("test_equals")
{
    json::variant var1(semantic_tag::none);
    json::variant var2{ json::object(), semantic_tag::none };
    CHECK((var1 == var1 && var2 == var2));
    CHECK((var1 == var2 && var2 == var1));

    json::variant var3{semantic_tag::none };
    CHECK((var3 == var1 && var1 == var3));
    json::variant var4{ json::object({{"first",1},{"second",2}}), semantic_tag::none };
    json::variant var5{ json::object({ { "first",1 },{ "second",2 } }), semantic_tag::none };
    CHECK((var3 != var4 && var4 != var3));
    CHECK((var2 != var4 && var4 != var2));
    CHECK(var4 == var4);
    CHECK(var4 == var5);
    CHECK(var5 == var4);

    json::variant var6(int64_t(100), semantic_tag::none);
    json::variant var7(uint64_t(100), semantic_tag::none);
    CHECK((var6 == var7 && var7 == var6));

    json::variant var8(100.0, semantic_tag::none);
    CHECK((var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7));

    std::string val9("small string");
    std::string val11("small string 2");
    json::variant var9(val9.data(), val9.length(), semantic_tag::none);
    json::variant var10(val9.data(),val9.length(), semantic_tag::none);
    json::variant var11(val11.data(),val11.length(), semantic_tag::none);

    std::string val12("too long for small string");
    std::string val14("too long for small string 2");
    json::variant var12(val12.data(),val12.length(), semantic_tag::none);
    json::variant var13(val12.data(),val12.length(), semantic_tag::none);
    json::variant var14(val14.data(),val14.length(), semantic_tag::none);
    CHECK((var9 == var10 && var10 == var9));
    CHECK((var9 != var11 && var11 != var9));
    CHECK((var12 == var13 && var13 == var12));
    CHECK((var12 != var14 && var14 != var12));

    json::variant var15(val9.data(),val9.length(), semantic_tag::none, std::allocator<char>());
    CHECK((var9 == var15 && var15 == var9));

    json::variant var16(static_cast<int64_t>(0), semantic_tag::none);
    json::variant var17(static_cast<uint64_t>(0), semantic_tag::none);
    CHECK(var16 == var17);
    CHECK(var17 == var16);
}

