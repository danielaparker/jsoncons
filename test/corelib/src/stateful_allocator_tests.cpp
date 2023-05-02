// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <common/FreeListAllocator.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR)
TEST_CASE("test stateful allocator")
{
    FreeListAllocator<char> alloc(true); 

    using custom_json = basic_json<char,sorted_policy,FreeListAllocator<char>>;

    SECTION("construct from string")
    {
        custom_json j("String too long for short string", alloc);
        CHECK(j.as<std::string>() == "String too long for short string");
    }

    SECTION("emplace_back")
    {
        custom_json j(json_array_arg, alloc);
        j.emplace_back(1);
        j.emplace_back("String too long for short string", alloc);

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == "String too long for short string");
    }

    SECTION("push_back")
    {
        custom_json j(json_array_arg, alloc);
        j.push_back(1);
        j.push_back(custom_json("String too long for short string", alloc));

        CHECK(j.size() == 2);
        CHECK(j.at(0) == 1);
        CHECK(j.at(1).as<std::string>() == "String too long for short string");
    }

    SECTION("parse")
    {
        FreeListAllocator<char> alloc2(true); 

        std::string s = "String too long for short string";
        std::string input = "\"" + s + "\"";

        json_decoder<custom_json,FreeListAllocator<char>> decoder(result_allocator_arg, alloc, alloc2);
        JSONCONS_TRY
        {
            json_string_reader reader(input,decoder);
            reader.read_next();
        }
        JSONCONS_CATCH (const std::exception&)
        {
        }
        CHECK(decoder.is_valid());
        auto j = decoder.get_result();
        CHECK(j.as<std::string>() == s);
    }

}
#endif

