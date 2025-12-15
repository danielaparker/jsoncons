// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json_tokenizer.hpp>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

/*TEST_CASE("json_tokenizer constructor test")
{
    SECTION("default constructor")
    {
        json_tokenizer tokenizer{};
        std::string str = R"(
{
    "foo" : "bar"

}
)";

        jsoncons::from_json_result r;
        tokenizer.update(str.data(), str.size());
        while (!tokenizer.done())
        {
            if (!r)
            {
                std::cout << (int)r.ec << "\n";
                //tokenizer.update("}", 1);
                //break;
            }
            else
            {
                std::cout << tokenizer.token_kind() << "\n";
            }
            r = tokenizer.try_next();
        }
    }
}*/

TEST_CASE("json_tokenizer update test")
{
    SECTION("empty input")
    {
        std::string data{};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK(tokenizer.done());
    }
    SECTION("input with whitespace")
    {
        std::string data{" "};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK(tokenizer.done());
    }
    SECTION("string")
    {
        std::string data{R"("Hello World")"};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        auto sv = tokenizer.get_string_view();
        CHECK(data.substr(1,data.size()-2) == sv);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("uint64_t max")
    {
        std::string data = std::to_string((std::numeric_limits<uint64_t>::max)());

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind{} == tokenizer.token_kind()); // still unknown, may be more data
        CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
        CHECK(generic_token_kind::uint64_value == tokenizer.token_kind());
        auto val = tokenizer.get_uint64_value();
        CHECK(data == std::to_string(val));
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("uint64_t max + space")
    {
        std::string data = std::to_string((std::numeric_limits<uint64_t>::max)()) + " ";

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::uint64_value == tokenizer.token_kind());
        auto val = tokenizer.get_uint64_value();
        CHECK(data.substr(0,data.size()-1) == std::to_string(val));
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("int64_t min")
    {
        std::string data = std::to_string((std::numeric_limits<int64_t>::lowest)());

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind{} == tokenizer.token_kind()); // still unknown, may be more data
        CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
        CHECK(generic_token_kind::int64_value == tokenizer.token_kind());
        auto val = tokenizer.get_int64_value();
        CHECK(data == std::to_string(val));
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("double max")
    {
        std::string data = std::to_string((std::numeric_limits<double>::max)());

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind{} == tokenizer.token_kind()); // still unknown, may be more data
        CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
        CHECK(generic_token_kind::double_value == tokenizer.token_kind());
        auto val = tokenizer.get_double_value();
        CHECK(data == std::to_string(val));
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("double min")
    {
        std::string data = std::to_string((std::numeric_limits<double>::lowest)());

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind{} == tokenizer.token_kind()); // still unknown, may be more data
        CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
        CHECK(generic_token_kind::double_value == tokenizer.token_kind());
        auto val = tokenizer.get_double_value();
        CHECK(data == std::to_string(val));
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("bool true")
    {
        std::string data{"true"};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::bool_value == tokenizer.token_kind());
        auto val = tokenizer.get_bool_value();
        CHECK(val);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("bool false")
    {
        std::string data{"false"};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::bool_value == tokenizer.token_kind());
        auto val = tokenizer.get_bool_value();
        CHECK_FALSE(val);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("null")
    {
        std::string data{"false"};

        json_tokenizer tokenizer{};
        CHECK(json_errc{} == tokenizer.try_update(data).ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::bool_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }

    SECTION("object with three members")
    {
        std::string data = R"({"A":"Jane", "B":"Roe", "C":10})";

        json_tokenizer tokenizer{};
        REQUIRE(json_errc{} == tokenizer.try_update(data).ec);
        CHECK(generic_token_kind::begin_map == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::uint64_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::end_map == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }

    SECTION("test1")
    {
        std::string data(R"({"foo":[42,null]})");

        json_tokenizer tokenizer{};
        REQUIRE(json_errc{} == tokenizer.try_update(data).ec);
        CHECK(generic_token_kind::begin_map == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::string_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::begin_array == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::uint64_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::null_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::end_array == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(generic_token_kind::end_map == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
}

TEST_CASE("json_tokenizer incremental update tests")
{
    SECTION("test 1")
    {
        std::string data{"123456"};
        std::string more_data{"78"};
        std::string no_data{""};

        json_tokenizer tokenizer{};
        REQUIRE(json_errc{} == tokenizer.try_update(data).ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        REQUIRE(json_errc{} == tokenizer.try_update(more_data).ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        REQUIRE(json_errc{} == tokenizer.try_update(no_data).ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(generic_token_kind::uint64_value == tokenizer.token_kind());
        CHECK(12345678 == tokenizer.get_uint64_value());
        REQUIRE(!tokenizer.done());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(tokenizer.done());
    }
}


