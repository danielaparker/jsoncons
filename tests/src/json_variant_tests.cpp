// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <new>
#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_variant_tests)

BOOST_AUTO_TEST_CASE(test_variant)
{
    json::variant var1(int64_t(-100));
    BOOST_CHECK(json_major_type::integer_t == var1.major_type());
    json::variant var2(uint64_t(100));
    BOOST_CHECK(json_major_type::uinteger_t == var2.major_type());
    json::variant var3("Small string",12);
    BOOST_CHECK(json_major_type::short_string_t == var3.major_type());
    json::variant var4("Too long to fit in small string",31);
    BOOST_CHECK(json_major_type::long_string_t == var4.major_type());
    json::variant var5(true);
    BOOST_CHECK(json_major_type::bool_t == var5.major_type());
    json::variant var6;
    BOOST_CHECK(json_major_type::empty_object_t == var6.major_type());
    json::variant var7{ null_type() };
    BOOST_CHECK(json_major_type::null_t == var7.major_type());
    json::variant var8{ json::allocator_type() };
    BOOST_CHECK(json_major_type::object_t == var8.major_type());
    json::variant var9(123456789.9);
    BOOST_CHECK(json_major_type::double_t == var9.major_type());
}

BOOST_AUTO_TEST_CASE(test_move_constructor)
{
    int64_t val1 = -100;
    json::variant var1(val1);
    json::variant var2(std::move(var1));
    //BOOST_CHECK(json_major_type::null_t == var1.major_type());
    BOOST_CHECK(json_major_type::integer_t == var2.major_type());
    BOOST_CHECK(var2.integer_data_cast()->value() == val1);

    uint64_t val3 = 9999;
    json::variant var3(val3);
    json::variant var4(std::move(var3));
    //BOOST_CHECK(json_major_type::null_t == var3.major_type());
    BOOST_CHECK(json_major_type::uinteger_t == var4.major_type());
    BOOST_CHECK(var4.uinteger_data_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5);
    json::variant var6(std::move(var5));
    //BOOST_CHECK(json_major_type::null_t == var5.major_type());
    BOOST_CHECK(json_major_type::double_t == var6.major_type());
    BOOST_CHECK(var6.double_data_cast()->value() == val5);

    std::string val7("Too long for small string");
    json::variant var7(val7.data(),val7.length());
    json::variant var8(std::move(var7));
    //BOOST_CHECK(json_major_type::null_t == var7.major_type());
    BOOST_CHECK(json_major_type::long_string_t == var8.major_type());
    BOOST_CHECK(val7 == var8.string_data_cast()->data());
    BOOST_CHECK(val7.length() == var8.string_data_cast()->length());

    std::string val9("Small string");
    json::variant var9(val9.data(), val9.length());
    json::variant var10(std::move(var9));
    //BOOST_CHECK(json_major_type::null_t == var9.major_type());
    BOOST_CHECK(json_major_type::short_string_t == var10.major_type());
    BOOST_CHECK(val9 == var10.short_string_data_cast()->data());
    BOOST_CHECK(val9.length() == var10.short_string_data_cast()->length());

    bool val11 = true;
    json::variant var11(val11);
    json::variant var12(std::move(var11));
    //BOOST_CHECK(json_major_type::null_t == var11.major_type());
    BOOST_CHECK(json_major_type::bool_t == var12.major_type());
    BOOST_CHECK(var12.bool_data_cast()->value() == val11);

    std::string val13("Too long for small string");
    json::variant var13(val13.data(), val13.length());
    json::variant var14(std::move(var13));
    //BOOST_CHECK(json_major_type::null_t == var13.major_type());
    BOOST_CHECK(json_major_type::long_string_t == var14.major_type());
    BOOST_CHECK(val13 == var14.string_data_cast()->data());

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15);
    json::variant var16(std::move(var15));
    BOOST_CHECK(json_major_type::null_t == var15.major_type());
    BOOST_CHECK(json_major_type::object_t == var16.major_type());
    BOOST_CHECK(val15 == var16.object_data_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17);
    json::variant var18(std::move(var17));
    BOOST_CHECK(json_major_type::null_t == var17.major_type());
    BOOST_CHECK(json_major_type::array_t == var18.major_type());
    BOOST_CHECK(val17 == var18.array_data_cast()->value());
}

BOOST_AUTO_TEST_CASE(test_copy_constructor)
{
    int64_t val1 = 123456789;
    json::variant var1(val1);
    json::variant var2(var1);
    BOOST_CHECK(json_major_type::integer_t == var1.major_type());
    BOOST_CHECK(json_major_type::integer_t == var2.major_type());
    BOOST_CHECK(var2.integer_data_cast()->value() == val1);

    uint64_t val3 = 123456789;
    json::variant var3(val3);
    json::variant var4(var3);
    BOOST_CHECK(json_major_type::uinteger_t == var3.major_type());
    BOOST_CHECK(json_major_type::uinteger_t == var4.major_type());
    BOOST_CHECK(var4.uinteger_data_cast()->value() == val3);

    double val5 = 123456789.9;
    json::variant var5(val5,floating_point_options());
    json::variant var6(var5);
    BOOST_CHECK(json_major_type::double_t == var5.major_type());
    BOOST_CHECK(json_major_type::double_t == var6.major_type());
    BOOST_CHECK(var6.double_data_cast()->value() == val5);

    std::string val9 = "Small string";
    json::variant var9(val9.data(),val9.length());
    json::variant var10(var9);
    BOOST_CHECK(json_major_type::short_string_t == var9.major_type());
    BOOST_CHECK(json_major_type::short_string_t == var10.major_type());
    BOOST_CHECK(var10.short_string_data_cast()->data() == val9);

    bool val11 = true;
    json::variant var11(val11);
    json::variant var12(var11);
    BOOST_CHECK(json_major_type::bool_t == var11.major_type());
    BOOST_CHECK(json_major_type::bool_t == var12.major_type());
    BOOST_CHECK(var12.bool_data_cast()->value() == val11);

    std::string val13 = "Too long for small string";
    json::variant var13(val13.data(),val13.length());
    json::variant var14(var13);
    BOOST_CHECK(json_major_type::long_string_t == var13.major_type());
    BOOST_CHECK(json_major_type::long_string_t == var14.major_type());
    BOOST_CHECK(var14.string_data_cast()->data() == val13);

    json::object val15 = { {"first",1},{"second",2} };
    json::variant var15(val15);
    json::variant var16(var15);
    BOOST_CHECK(json_major_type::object_t == var15.major_type());
    BOOST_CHECK(json_major_type::object_t == var16.major_type());
    BOOST_CHECK(val15 == var16.object_data_cast()->value());

    json::array val17 = {1,2,3,4};
    json::variant var17(val17);
    json::variant var18(var17);
    BOOST_CHECK(json_major_type::array_t == var17.major_type());
    BOOST_CHECK(json_major_type::array_t == var18.major_type());
    BOOST_CHECK(val17 == var18.array_data_cast()->value());
}

BOOST_AUTO_TEST_CASE(test_equals)
{
    json::variant var1;
    json::variant var2{ json::object() };
    BOOST_CHECK(var1 == var1 && var2 == var2);
    BOOST_CHECK(var1 == var2 && var2 == var1);

    json::variant var3;
    BOOST_CHECK(var3 == var1 && var1 == var3);
    json::variant var4{ json::object({{"first",1},{"second",2}}) };
    json::variant var5{ json::object({ { "first",1 },{ "second",2 } }) };
    BOOST_CHECK(var3 != var4 && var4 != var3);
    BOOST_CHECK(var2 != var4 && var4 != var2);
    BOOST_CHECK(var4 == var4);
    BOOST_CHECK(var4 == var5);
    BOOST_CHECK(var5 == var4);

    json::variant var6(int64_t(100));
    json::variant var7(uint64_t(100));
    BOOST_CHECK(var6 == var7 && var7 == var6);

    json::variant var8(100.0);
    BOOST_CHECK(var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7);

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
    BOOST_CHECK(var9 == var10 && var10 == var9);
    BOOST_CHECK(var9 != var11 && var11 != var9);
    BOOST_CHECK(var12 == var13 && var13 == var12);
    BOOST_CHECK(var12 != var14 && var14 != var12);

    json::variant var15(val9.data(),val9.length(),std::allocator<char>());
    BOOST_CHECK(var9 == var15 && var15 == var9);

    json::variant var16(static_cast<int64_t>(0));
    json::variant var17(static_cast<uint64_t>(0));
    BOOST_CHECK(var16 == var17);
    BOOST_CHECK(var17 == var16);
}

BOOST_AUTO_TEST_SUITE_END()

