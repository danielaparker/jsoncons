// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test_object_equals_basic")
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["c"] = 3;
    o2["a"] = 1;
    o2["b"] = 2;

    CHECK(o1 == o2);
    CHECK(o2 == o1);
    CHECK_FALSE((o1 != o2));
    CHECK_FALSE((o2 != o1));
}

TEST_CASE("test_object_equals_diff_vals")
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["a"] = 1;
    o2["b"] = 4;
    o2["c"] = 3;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_diff_el_names")
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["d"] = 1;
    o2["e"] = 2;
    o2["f"] = 3;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_diff_sizes")
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["a"] = 1;
    o2["b"] = 2;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_subtle_offsets")
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 1;

    json o2;
    o2["b"] = 1;
    o2["c"] = 1;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_empty_objects")
{
    json def_constructed_1;
    json def_constructed_2;
    json parsed_1 = json::parse("{}");
    json parsed_2 = json::parse("{}");
    json type_constructed_1 = json(json_object_arg);
    json type_constructed_2 = json(json_object_arg);

    CHECK(def_constructed_1 == def_constructed_1);
    CHECK(parsed_1 == parsed_2);
    CHECK(type_constructed_1 == type_constructed_2);

    CHECK(def_constructed_1 == parsed_1);
    CHECK(def_constructed_1 == type_constructed_1);
    CHECK(parsed_1 == type_constructed_1);
}

TEST_CASE("test_object_equals_empty_arrays")
{
    json parsed_1 = json::parse("[]");
    json parsed_2 = json::parse("[]");
    json type_constructed_1(json_array_arg);
    json type_constructed_2(json_array_arg);

    CHECK(parsed_1 == parsed_2);
    CHECK(type_constructed_1 == type_constructed_2);

    CHECK(parsed_1 == type_constructed_1);
}

TEST_CASE("test_empty_object_equal")
{
    CHECK(json() == json(json_object_arg));
    CHECK(json(json_object_arg) == json());
}

TEST_CASE("test_string_not_equals_empty_object")
{
    json o1("42");
    json o2;

    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_byte_strings_equal")
{
    json o1(byte_string({'1','2','3','4','5','6','7','8','9'}));
    json o2(byte_string{'1','2','3','4','5','6','7','8','9'});
    json o3(byte_string{'1','2','3','4','5','6','7','8'});

    CHECK(o1 == o2);
    CHECK(o2 == o1);
    CHECK(o3 != o1);
    CHECK(o2 != o3);
}

TEST_CASE("json comparator equals tests")
{
    json var1(semantic_tag::none);
    json var2{ json::object(), semantic_tag::none };
    CHECK((var1 == var1 && var2 == var2));
    CHECK((var1 == var2 && var2 == var1));

    json var3{semantic_tag::none };
    CHECK((var3 == var1 && var1 == var3));
    json var4{ json::object({{"first",1},{"second",2}}), semantic_tag::none };
    json var5{ json::object({ { "first",1 },{ "second",2 } }), semantic_tag::none };
    CHECK((var3 != var4 && var4 != var3));
    CHECK((var2 != var4 && var4 != var2));
    CHECK(var4 == var4);
    CHECK(var4 == var5);
    CHECK(var5 == var4);

    json var6(int64_t(100), semantic_tag::none);
    json var7(uint64_t(100), semantic_tag::none);
    CHECK((var6 == var7 && var7 == var6));

    json var8(100.0, semantic_tag::none);
    CHECK((var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7));

    std::string val9("small string");
    std::string val11("small string 2");
    json var9(val9.data(), val9.length(), semantic_tag::none);
    json var10(val9.data(),val9.length(), semantic_tag::none);
    json var11(val11.data(),val11.length(), semantic_tag::none);

    std::string val12("too long for small string");
    std::string val14("too long for small string 2");
    json var12(val12.data(),val12.length(), semantic_tag::none);
    json var13(val12.data(),val12.length(), semantic_tag::none);
    json var14(val14.data(),val14.length(), semantic_tag::none);
    CHECK((var9 == var10 && var10 == var9));
    CHECK((var9 != var11 && var11 != var9));
    CHECK((var12 == var13 && var13 == var12));
    CHECK((var12 != var14 && var14 != var12));

    json var15(val9.data(),val9.length(), semantic_tag::none, std::allocator<char>());
    CHECK((var9 == var15 && var15 == var9));

    json var16(static_cast<int64_t>(0), semantic_tag::none);
    json var17(static_cast<uint64_t>(0), semantic_tag::none);
    CHECK(var16 == var17);
    CHECK(var17 == var16);
}


