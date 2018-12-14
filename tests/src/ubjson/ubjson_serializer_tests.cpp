// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::ubjson;

TEST_CASE("serialize array to ubjson")
{
    std::vector<uint8_t> v;
    ubjson_buffer_serializer serializer(v);
    //serializer.begin_object(1);
    serializer.begin_array(3);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.end_array();
    //serializer.end_object();
    serializer.flush();

    try
    {
        json result = decode_ubjson<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("Too many and too few items in UBJSON object or array")
{
    bool result;
    std::error_code ec{};
    std::vector<uint8_t> v;
    ubjson_buffer_serializer serializer(v);

    SECTION("Too many items in array")
    {
        serializer.begin_array(3);
        serializer.bool_value(true);
        serializer.bool_value(false);
        serializer.null_value();
        serializer.begin_array(2);
        serializer.string_value("cat");
        serializer.string_value("feline");
        serializer.end_array();
        result = serializer.end_array(null_serializing_context(), ec);
        CHECK(ec == ubjson_errc::too_many_items);
        CHECK_FALSE(result);
        serializer.flush();
    }
    SECTION("Too few items in array")
    {
        serializer.begin_array(5);
        serializer.bool_value(true);
        serializer.bool_value(false);
        serializer.null_value();
        serializer.begin_array(2);
        serializer.string_value("cat");
        serializer.string_value("feline");
        serializer.end_array();
        result = serializer.end_array(null_serializing_context(), ec);
        CHECK(ec == ubjson_errc::too_few_items);
        CHECK_FALSE(result);
        serializer.flush();
    }
    SECTION("Too many items in object")
    {
        serializer.begin_object(3);
        serializer.name("a");
        serializer.bool_value(true);
        serializer.name("b");
        serializer.bool_value(false);
        serializer.name("c");
        serializer.null_value();
        serializer.name("d");
        serializer.begin_array(2);
        serializer.string_value("cat");
        serializer.string_value("feline");
        serializer.end_array();
        result = serializer.end_array(null_serializing_context(), ec);
        CHECK(ec == ubjson_errc::too_many_items);
        CHECK_FALSE(result);
        serializer.flush();
    }
    SECTION("Too few items in object")
    {
        serializer.begin_object(5);
        serializer.name("a");
        serializer.bool_value(true);
        serializer.name("b");
        serializer.bool_value(false);
        serializer.name("c");
        serializer.null_value();
        serializer.name("d");
        serializer.begin_array(2);
        serializer.string_value("cat");
        serializer.string_value("feline");
        serializer.end_array();
        result = serializer.end_array(null_serializing_context(), ec);
        CHECK(ec == ubjson_errc::too_few_items);
        CHECK_FALSE(result);
        serializer.flush();
    }
}
