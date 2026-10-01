// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("basic_json object == jsoncons::basic_json object")
{
    SECTION("empty, empty")
    {
        jsoncons::json o1;
        jsoncons::json o2;
        jsoncons::json o3(jsoncons::json_object_arg);

        CHECK(o1 == o2);
        CHECK(o2 == o1);
        CHECK(o1 == o3);

        CHECK(o1 >= o2);
        CHECK(o2 <= o1);
        CHECK(o1 >= o3);
        CHECK(o3 >= o2);

        CHECK_FALSE((o1 != o2));
        CHECK_FALSE((o2 != o1));
        CHECK_FALSE((o1 != o3));

        CHECK_FALSE(o1 < o2);
        CHECK_FALSE(o2 < o1);
        CHECK_FALSE(o1 < o3);
        CHECK_FALSE(o1 > o2);
        CHECK_FALSE(o2 > o1);
        CHECK_FALSE(o1 > o3);
    }

    SECTION("empty and nonempty")
    {
        jsoncons::json a;
        a["c"] = 3;
        a["a"] = 1;
        a["b"] = 2;

        jsoncons::json b;

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }

    SECTION("nonempty and shorter")
    {
        jsoncons::json a;
        a["a"] = "hello";
        a["b"] = 1.0;
        a["c"] = true;

        jsoncons::json b;
        b["a"] = "hello";
        b["b"] = 1.0;

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }

    SECTION("nonempty and different")
    {
        jsoncons::json o1;
        o1["a"] = 1;
        o1["b"] = 2;
        o1["c"] = 3;

        jsoncons::json o2;
        o2["c"] = 3;
        o2["a"] = 1;
        o2["b"] = 2;

        CHECK(o1 == o2);
        CHECK(o2 == o1);
        CHECK_FALSE((o1 != o2));
        CHECK_FALSE((o2 != o1));

        CHECK(std::is_convertible<decltype(o1.at("a")),jsoncons::json>::value);
        CHECK(jsoncons::nonstd::is_basic_json<decltype(o1.at("a"))>::value);
        CHECK(jsoncons::nonstd::is_basic_json<const jsoncons::json&>::value);

        CHECK(1 == (o1.at("a"))); // jsoncons::basic_json == int
        CHECK((1 == o1.at("a"))); // int == jsoncons::basic_json
        CHECK(1 == (o1["a"]));    // proxy == int
        CHECK((1 == o1["a"]));    // int == proxy

        CHECK((o1.at("b") != 1)); // jsoncons::basic_json == int
        CHECK((1 != o1.at("b"))); // int == jsoncons::basic_json
        CHECK((o1["b"] != 1));    // proxy == int
        CHECK((1 != o1["b"]));    // int == proxy
    }
}

TEST_CASE("basic_json == jsoncons::basic_json")
{
    SECTION("test 1")
    {
        jsoncons::json o1;
        o1["a"] = 1;
        o1["b"] = 2;

        jsoncons::json o2(2);

        CHECK_FALSE((o1["a"] == o2));
        CHECK_FALSE((o2 == o1["a"]));
        CHECK((o1["a"] == o1["a"]));
        CHECK_FALSE(o1["a"] == o1["b"]);
        CHECK(o1["b"] == o2);
        CHECK((o2 == o1["b"]));
    }
}

TEST_CASE("test_object_equals_diff_vals")
{
    jsoncons::json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    jsoncons::json o2;
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
    jsoncons::json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    jsoncons::json o2;
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
    jsoncons::json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    jsoncons::json o2;
    o2["a"] = 1;
    o2["b"] = 2;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_subtle_offsets")
{
    jsoncons::json o1;
    o1["a"] = 1;
    o1["b"] = 1;

    jsoncons::json o2;
    o2["b"] = 1;
    o2["c"] = 1;

    CHECK_FALSE((o1 == o2));
    CHECK_FALSE((o2 == o1));
    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_object_equals_empty_objects")
{
    jsoncons::json def_constructed_1;
    jsoncons::json def_constructed_2;
    jsoncons::json parsed_1 = jsoncons::json::parse("{}");
    jsoncons::json parsed_2 = jsoncons::json::parse("{}");
    jsoncons::json type_constructed_1 = jsoncons::json(jsoncons::json_object_arg);
    jsoncons::json type_constructed_2 = jsoncons::json(jsoncons::json_object_arg);

    CHECK(def_constructed_1 == def_constructed_1);
    CHECK(parsed_1 == parsed_2);
    CHECK(type_constructed_1 == type_constructed_2);

    CHECK(def_constructed_1 == parsed_1);
    CHECK(def_constructed_1 == type_constructed_1);
    CHECK(parsed_1 == type_constructed_1);
}

TEST_CASE("test_object_equals_empty_arrays")
{
    jsoncons::json parsed_1 = jsoncons::json::parse("[]");
    jsoncons::json parsed_2 = jsoncons::json::parse("[]");
    jsoncons::json type_constructed_1(jsoncons::json_array_arg);
    jsoncons::json type_constructed_2(jsoncons::json_array_arg);

    CHECK(parsed_1 == parsed_2);
    CHECK(type_constructed_1 == type_constructed_2);

    CHECK(parsed_1 == type_constructed_1);
}

TEST_CASE("test_empty_object_equal")
{
    CHECK(jsoncons::json() == jsoncons::json(jsoncons::json_object_arg));
    CHECK(jsoncons::json(jsoncons::json_object_arg) == jsoncons::json());
}

TEST_CASE("test_string_not_equals_empty_object")
{
    jsoncons::json o1("42");
    jsoncons::json o2;

    CHECK(o1 != o2);
    CHECK(o2 != o1);
}

TEST_CASE("test_byte_strings_equal")
{
    jsoncons::json o1(jsoncons::byte_string({'1','2','3','4','5','6','7','8','9'}));
    jsoncons::json o2(jsoncons::byte_string{'1','2','3','4','5','6','7','8','9'});
    jsoncons::json o3(jsoncons::byte_string{'1','2','3','4','5','6','7','8'});

    CHECK(o1 == o2);
    CHECK(o2 == o1);
    CHECK(o3 != o1);
    CHECK(o2 != o3);
}

TEST_CASE("json comparator equals tests")
{
    jsoncons::json j1(jsoncons::semantic_tag::none);
    jsoncons::json j2{ jsoncons::json::object(), jsoncons::semantic_tag::none };
    CHECK((j1 == j1 && j2 == j2));
    CHECK((j1 == j2 && j2 == j1));

    jsoncons::json var3{semantic_tag::none };
    CHECK((var3 == j1 && j1 == var3));
    jsoncons::json var4{ jsoncons::json::object({{"first",1},{"second",2}}), jsoncons::semantic_tag::none };
    jsoncons::json var5{ jsoncons::json::object({ { "first",1 },{ "second",2 } }), jsoncons::semantic_tag::none };
    CHECK((var3 != var4 && var4 != var3));
    CHECK((j2 != var4 && var4 != j2));
    CHECK(var4 == var4);
    CHECK(var4 == var5);
    CHECK(var5 == var4);

    jsoncons::json var6(int64_t(100), jsoncons::semantic_tag::none);
    jsoncons::json var7(uint64_t(100), jsoncons::semantic_tag::none);
    CHECK((var6 == var7 && var7 == var6));

    jsoncons::json var8(100.0, jsoncons::semantic_tag::none);
    CHECK((var8 == var8 && var6 == var8 && var8 == var6 && var7 == var8 && var8 == var7));

    std::string val9("small string");
    std::string val11("small string 2");
    jsoncons::json var9(val9.data(), val9.length(), jsoncons::semantic_tag::none);
    jsoncons::json var10(val9.data(),val9.length(), jsoncons::semantic_tag::none);
    jsoncons::json var11(val11.data(),val11.length(), jsoncons::semantic_tag::none);

    std::string val12("too long for small string");
    std::string val14("too long for small string 2");
    jsoncons::json var12(val12.data(),val12.length(), jsoncons::semantic_tag::none);
    jsoncons::json var13(val12.data(),val12.length(), jsoncons::semantic_tag::none);
    jsoncons::json var14(val14.data(),val14.length(), jsoncons::semantic_tag::none);
    CHECK((var9 == var10 && var10 == var9));
    CHECK((var9 != var11 && var11 != var9));
    CHECK((var12 == var13 && var13 == var12));
    CHECK((var12 != var14 && var14 != var12));

    jsoncons::json var15(val9.data(),val9.length(), jsoncons::semantic_tag::none, std::allocator<char>());
    CHECK((var9 == var15 && var15 == var9));

    jsoncons::json var16(static_cast<int64_t>(0), jsoncons::semantic_tag::none);
    jsoncons::json var17(static_cast<uint64_t>(0), jsoncons::semantic_tag::none);
    CHECK(var16 == var17);
    CHECK(var17 == var16);
}

TEST_CASE("basic_json number compare")
{
    SECTION("unsigned unsigned")
    {
        jsoncons::json o;
        o["a"] = std::numeric_limits<uint64_t>::max();
        o["b"] = std::numeric_limits<uint64_t>::lowest();

        CHECK(o.at("a") == o.at("a")); // value and value
        CHECK(o.at("a") == o["a"]); // value and proxy
        CHECK(o["a"] == o.at("a")); // proxy and value
        CHECK(o["a"] == o["a"]); // proxy and proxy

        CHECK(o.at("a") <= o.at("a")); // value and value
        CHECK(o.at("a") <= o["a"]); // value and proxy
        CHECK(o["a"] <= o.at("a")); // proxy and value
        CHECK(o["a"] <= o["a"]); // proxy and proxy

        CHECK(o.at("a") >= o.at("a")); // value and value
        CHECK(o.at("a") >= o["a"]); // value and proxy
        CHECK(o["a"] >= o.at("a")); // proxy and value
        CHECK(o["a"] >= o["a"]); // proxy and proxy

        CHECK(o.at("a") != o.at("b")); // value and value
        CHECK(o.at("a") != o["b"]); // value and proxy
        CHECK(o["a"] != o.at("b")); // proxy and value
        CHECK(o["a"] != o["b"]); // proxy and proxy

        CHECK(o.at("a") > o.at("b")); // value and value
        CHECK(o.at("a") > o["b"]); // value and proxy
        CHECK(o["a"] > o.at("b")); // proxy and value
        CHECK(o["a"] > o["b"]); // proxy and proxy

        CHECK(o.at("a") >= o.at("b")); // value and value
        CHECK(o.at("a") >= o["b"]); // value and proxy
        CHECK(o["a"] >= o.at("b")); // proxy and value
        CHECK(o["a"] >= o["b"]); // proxy and proxy

        CHECK_FALSE(o.at("a") < o.at("b")); // value and value
        CHECK_FALSE(o.at("a") < o["b"]); // value and proxy
        CHECK_FALSE(o["a"] < o.at("b")); // proxy and value
        CHECK_FALSE(o["a"] < o["b"]); // proxy and proxy

        CHECK_FALSE(o.at("a") <= o.at("b")); // value and value
        CHECK_FALSE(o.at("a") <= o["b"]); // value and proxy
        CHECK_FALSE(o["a"] <= o.at("b")); // proxy and value
        CHECK_FALSE(o["a"] <= o["b"]); // proxy and proxy
    }
    SECTION("signed signed test")
    {
        auto a = std::numeric_limits<int64_t>::max();
        auto b = std::numeric_limits<int64_t>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("unsigned signed test")
    {
        jsoncons::json a = std::numeric_limits<uint64_t>::max();
        jsoncons::json b = std::numeric_limits<int64_t>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("signed unsigned test")
    {
        jsoncons::json a = std::numeric_limits<int64_t>::max();
        jsoncons::json b = std::numeric_limits<uint64_t>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("double double test")
    {
        jsoncons::json a = std::numeric_limits<double>::max();
        jsoncons::json b = std::numeric_limits<double>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("signed double test")
    {
        jsoncons::json a = std::numeric_limits<int64_t>::max();
        jsoncons::json b = std::numeric_limits<double>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("double signed test")
    {
        jsoncons::json a = std::numeric_limits<double>::max();
        jsoncons::json b = std::numeric_limits<int64_t>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("unsigned double test")
    {
        jsoncons::json a = std::numeric_limits<uint64_t>::max();
        jsoncons::json b = std::numeric_limits<double>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("double unsigned test")
    {
        jsoncons::json a = std::numeric_limits<double>::max();
        jsoncons::json b = std::numeric_limits<uint64_t>::lowest();

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
    SECTION("double and bigint string")
    {
        jsoncons::json a{std::numeric_limits<double>::max()};
        jsoncons::json b{std::to_string(std::numeric_limits<uint64_t>::lowest()), jsoncons::semantic_tag::bigint};

        CHECK_FALSE(a == b); 
        CHECK(a != b);
        CHECK(a > b);
        CHECK(a >= b);
        CHECK(b < a);
        CHECK(b <= a);
        CHECK_FALSE(a < b); 
        CHECK_FALSE(a <= b);
        CHECK_FALSE(b > a); 
        CHECK_FALSE(b >= a);
    }
    SECTION("bigint string and double")
    {
        jsoncons::json a{ std::to_string(std::numeric_limits<uint64_t>::max()), jsoncons::semantic_tag::bigint };
        jsoncons::json b{ std::numeric_limits<double>::lowest() };

        CHECK_FALSE(a == b);
        CHECK(a != b);
        CHECK(a > b);
        CHECK(a >= b);
        CHECK(b < a);
        CHECK(b <= a);
        CHECK_FALSE(a < b);
        CHECK_FALSE(a <= b);
        CHECK_FALSE(b > a);
        CHECK_FALSE(b >= a);
    }
    SECTION("double and non-numeric string")
    {
        jsoncons::json a{ std::numeric_limits<double>::max() };
        jsoncons::json b{ "Hello world" };

        CHECK_FALSE(a == b);
        CHECK(a != b);
        CHECK_FALSE(a > b);
        CHECK_FALSE(a >= b);
        CHECK_FALSE(b < a);
        CHECK_FALSE(b <= a);
        CHECK(a < b);
        CHECK(a <= b);
        CHECK(b > a);
        CHECK(b >= a);
    }
    SECTION("non-numeric string and double")
    {
        jsoncons::json a{"Hello world"};
        jsoncons::json b{ std::numeric_limits<double>::lowest() };

        CHECK_FALSE(a == b);
        CHECK(a != b);
        CHECK(a > b);
        CHECK(a >= b);
        CHECK(b < a);
        CHECK(b <= a);
        CHECK_FALSE(a < b);
        CHECK_FALSE(a <= b);
        CHECK_FALSE(b > a);
        CHECK_FALSE(b >= a);
    }
}

TEST_CASE("basic_json bool comparator")
{
    SECTION("bool")
    {
        jsoncons::json a(true);
        jsoncons::json b(false);

        CHECK(a == a); // value and value
        CHECK(a <= a); // value and value
        CHECK(a >= a); // value and value
        CHECK(a != b); // value and value
        CHECK(a > b); // value and value
        CHECK(a >= b); // value and value
        CHECK(b < a); // value and value
        CHECK(b <= a); // value and value
        CHECK_FALSE(a < b); // value and value
        CHECK_FALSE(a <= b); // value and value
        CHECK_FALSE(b > a); // value and value
        CHECK_FALSE(b >= a); // value and value
    }
}

TEST_CASE("basic_json big number compare")
{
    SECTION("test1")
    {
        jsoncons::json j1 = jsoncons::json::parse("[1e999]");
        jsoncons::json j2 = j1;
        jsoncons::json j3 = j1;

        CHECK(j1 == j1);
        CHECK(j2 == j1);
        CHECK(j1 == j2);
        CHECK(j2 == j3);
        CHECK_FALSE(j1 < j2);
        CHECK_FALSE(j2 < j3);
        CHECK_FALSE(j2 < j1);
        CHECK_FALSE(j3 < j2);
    }
    SECTION("test2")
    {
        jsoncons::json j1 = jsoncons::json::parse("[1e999]");
        jsoncons::json j2 = jsoncons::json::parse("[-1e999]");

        CHECK_FALSE(j1 == j2);
        CHECK(j1 > j2);
        CHECK_FALSE(j2 > j1);
        CHECK_FALSE(j1 < j2);
        CHECK(j2 < j1);
    }
    SECTION("test3")
    {
        jsoncons::json j1 = jsoncons::json::parse("[1e999]");
        jsoncons::json j2 = jsoncons::json::parse("1000");

        CHECK_FALSE(j1 == j2);
        CHECK(j1 > j2);
        CHECK_FALSE(j2 > j1);
        CHECK_FALSE(j1 < j2);
        CHECK(j2 < j1);
    }
}

