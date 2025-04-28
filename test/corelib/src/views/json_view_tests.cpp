#include <catch/catch.hpp>
#include <jsoncons/views/json_container.hpp>
#include <iostream>

using jsoncons::json_container;
using jsoncons::json_view;
using jsoncons::json_type;
using jsoncons::semantic_tag;
using jsoncons::read_json_flags;

TEST_CASE("json_view test")
{
    std::string str = R"(
{
    "books":
    [
        {
            "category": "fiction",
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "category": "fiction",
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "category": "fiction",
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "category": "memoir",
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
    )";
    
    auto doc = json_container<>::parse(str);

    json_view root = doc->root();
    
    REQUIRE(root.is_object());
    REQUIRE(root.size() == 1);
    REQUIRE_FALSE(root.empty());   
    REQUIRE(root["books"].size() == 4);
    REQUIRE(root.at("books").size() == 4);

}

TEST_CASE("json_view type test")
{
    std::string str = R"(
{
  "array": [0,-1,-2,3,4,5,"test",123.456,true,false,null,{"a":1,"b":2,"c":3}],
  "object": {
    "key0": 0,
    "key1": 1,
    "key2": 2,
    "key3": true,
    "key4": false,
    "key5": null,
    "key6": [1,2,3,4,5,"test",123.456,true,false,null],
    "key7": {"a":1,"b":2,"c":3}
  }
}
    )";
    SECTION("default")
    {
        auto doc = json_container<>::parse(str);

        json_view root = doc->root();

        REQUIRE(root.is_object());
        REQUIRE(root.size() == 2);
        REQUIRE_FALSE(root.empty());   

        auto array = root["array"];
        REQUIRE(array.is_array());
        REQUIRE(array.size() == 12);
        REQUIRE(array[1].cast<int>() == -1);
        REQUIRE(array[2].cast<int>() == -2);
        REQUIRE(array[5].cast<int>() == 5);
        REQUIRE(array[6].is_string());
        REQUIRE(array[6].get_string_view() == "test");
        REQUIRE(array[6].get_cstring() == std::string("test"));
        REQUIRE(array[7].get_double() == 123.456);
        REQUIRE(array[8].is_bool());
        REQUIRE(array[8].get_bool() == true);
        REQUIRE(array[9].is_bool());
        REQUIRE(array[9].get_bool() == false);
        REQUIRE(array[10].is_null());

        auto object = root["object"];
        REQUIRE(object.is_object());
        REQUIRE(object.size() == 8);
        REQUIRE(object["key3"].is_bool());
        REQUIRE(object["key3"].get_bool() == true);
        REQUIRE(object["key4"].is_bool());
        REQUIRE(object["key4"].get_bool() == false);
        REQUIRE(object["key5"].is_null());
    }
    SECTION("raw number")
    {
        auto doc = json_container<>::parse(str, read_json_flags::number_as_raw);

        json_view root = doc->root();

        REQUIRE(root.is_object());
        REQUIRE(root.size() == 2);
        REQUIRE_FALSE(root.empty());   

        auto array = root["array"];
        REQUIRE(array.is_array());
        REQUIRE(array.size() == 12);
        REQUIRE(array[1].is_string());
        REQUIRE(array[1].cast<int>() == -1);
        CHECK(array[1].tag() == semantic_tag::bigint);
        REQUIRE(array[5].is_string());
        REQUIRE(array[5].cast<int>() == 5);
        REQUIRE(array[6].is_string());
        REQUIRE(array[6].get_string_view() == "test");
        REQUIRE(array[6].get_cstring() == std::string("test"));
        REQUIRE(array[7].type() == json_type::string_value);
        REQUIRE(array[7].get_double() == 123.456);
        REQUIRE(array[8].is_bool());
        REQUIRE(array[8].get_bool() == true);
        REQUIRE(array[9].is_bool());
        REQUIRE(array[9].get_bool() == false);
        REQUIRE(array[10].is_null());

        auto object = root["object"];
        REQUIRE(object.is_object());
        REQUIRE(object.size() == 8);
        REQUIRE(object["key3"].is_bool());
        REQUIRE(object["key3"].get_bool() == true);
        REQUIRE(object["key4"].is_bool());
        REQUIRE(object["key4"].get_bool() == false);
        REQUIRE(object["key5"].is_null());
    }
}

TEST_CASE("test json_view inf and nan")
{
    std::string str = R"(
[NAN,NaN,Nan,naN,nan,-NAN,-NaN,-Nan,-naN,-nan,INF,Inf,inF,inf,INFINITY,Infinity,infINITY,infinity,-INF,-Inf,-inF,-inf,-INFINITY,-Infinity,-infINITY,-infinity]
    )";
    SECTION("regular number")
    {
        auto result = json_container<>::parse(str, read_json_flags::allow_inf_and_nan);

        json_view root = result->root();

        REQUIRE(root.is_array());
        //REQUIRE(root.size() == 9);
        for (const auto& item : root.array_range())
        {
            CHECK(item.is_double());
        }
    }
    SECTION("raw number")
    {
        auto result = json_container<>::parse(str, read_json_flags::allow_inf_and_nan | read_json_flags::number_as_raw);

        json_view root = result->root();

        REQUIRE(root.is_array());
        //REQUIRE(root.size() == 12);
        for (const auto& item : root.array_range())
        {
            CHECK(item.is_string());
            CHECK(item.tag() == semantic_tag::bigdec);
        }
        CHECK(root[0].get_string_view() == "NAN");
        CHECK(root[1].get_string_view() == "NaN");
    }
    SECTION("single nan")
    {
        auto result = json_container<>::parse("nan", read_json_flags::allow_inf_and_nan);

        json_view root = result->root();

        REQUIRE(root.is_double());
    }
    SECTION("raw single nan")
    {
        auto result = json_container<>::parse("nan", read_json_flags::allow_inf_and_nan | read_json_flags::number_as_raw);

        json_view root = result->root();

        REQUIRE(root.is_string());
        CHECK(root.tag() == semantic_tag::bigdec);
        CHECK(root.get_string_view() == "nan");
    }
    SECTION("single inf")
    {
        auto result = json_container<>::parse("inf", read_json_flags::allow_inf_and_nan);

        json_view root = result->root();

        REQUIRE(root.is_double());
    }
    SECTION("raw single inf")
    {
        auto result = json_container<>::parse("inf", read_json_flags::allow_inf_and_nan | read_json_flags::number_as_raw);

        json_view root = result->root();

        REQUIRE(root.is_string());
        CHECK(root.tag() == semantic_tag::bigdec);
        CHECK(root.get_string_view() == "inf");
    }
}

