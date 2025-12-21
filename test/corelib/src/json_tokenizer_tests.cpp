// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json_tokenizer.hpp>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;
#if 0
TEST_CASE("json_tokenizer constructor test")
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
}

TEST_CASE("json_tokenizer update test")
{
    SECTION("empty input")
    {
        std::string data{};

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("input with whitespace")
    {
        std::string data{" "};

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }
    SECTION("string")
    {
        std::string data{R"("Hello World")"};

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind::bool_value == tokenizer.token_kind());
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }

    SECTION("object with three members")
    {
        std::string data = R"({"A":"Jane", "B":"Roe", "C":10})";

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
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
        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
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

    SECTION("double")
    {
        double expected = 42.229999999999997;
        std::string data = std::to_string(expected);

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK_FALSE(tokenizer.done());
        CHECK(generic_token_kind{} == tokenizer.token_kind()); // still unknown, may be more data
        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
        }
        CHECK_FALSE(tokenizer.done());
        CHECK(tokenizer.source_exhausted());
        CHECK(generic_token_kind{} == tokenizer.token_kind());

        if (!tokenizer.done())
        {
            CHECK(json_errc{} == tokenizer.try_next().ec);  // now we know we have the number
            CHECK(generic_token_kind::double_value == tokenizer.token_kind());
        }
        auto val = tokenizer.get_double_value();
        CHECK(expected == val);
        CHECK(json_errc{} == tokenizer.try_next().ec);
        CHECK(tokenizer.done());
    }

    SECTION("array")
    {
        std::string data = "[1,2,3]";
        json_tokenizer tokenizer;

        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);

        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << tokenizer.get_string_view() << "\n";
                    break;
                case generic_token_kind::uint64_value:
                    std::cout << tokenizer.get_uint64_value() << "\n";
                    break;
                case generic_token_kind::begin_array:
                    std::cout << "begin_array\n";
                    break;
                case generic_token_kind::end_array:
                    std::cout << "end_array\n";
                    break;
                case generic_token_kind::begin_map:
                    std::cout << "begin_map\n";
                    break;
                case generic_token_kind::end_map:
                    std::cout << "end_map\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
        while (!tokenizer.done())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << tokenizer.get_string_view() << "\n";
                    break;
                case generic_token_kind::uint64_value:
                    std::cout << tokenizer.get_uint64_value() << "\n";
                    break;
                case generic_token_kind::begin_array:
                    std::cout << "begin_array\n";
                    break;
                case generic_token_kind::end_array:
                    std::cout << "end_array\n";
                    break;
                case generic_token_kind::begin_map:
                    std::cout << "begin_map\n";
                    break;
                case generic_token_kind::end_map:
                    std::cout << "end_map\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
    }
}
#endif

TEST_CASE("json_tokenizer incremental update tests")
{
    /*SECTION("test 1")
    {
        std::string data{"123456"};
        std::string more_data{"78"};
        std::string no_data{""};

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        tokenizer.update(more_data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        tokenizer.update(no_data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind::uint64_value == tokenizer.token_kind());
        CHECK(12345678 == tokenizer.get_uint64_value());
        REQUIRE(!tokenizer.done());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(tokenizer.done());
    }
    SECTION("test 2")
    {
        std::string data{"[123456"};
        std::string more_data{"78"};
        std::string last_data{"]"};

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(!tokenizer.source_exhausted());
        REQUIRE(generic_token_kind::begin_array == tokenizer.token_kind());
        REQUIRE(generic_token_kind::begin_array == tokenizer.token_kind());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        tokenizer.update(more_data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(tokenizer.source_exhausted());
        REQUIRE(generic_token_kind{} == tokenizer.token_kind());
        tokenizer.update(last_data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(!tokenizer.done());
        REQUIRE(!tokenizer.source_exhausted()); // 
        REQUIRE(generic_token_kind::uint64_value == tokenizer.token_kind());
        CHECK(12345678 == tokenizer.get_uint64_value());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(generic_token_kind::end_array == tokenizer.token_kind());
        REQUIRE(!tokenizer.done());
        REQUIRE(json_errc{} == tokenizer.try_next().ec);
        REQUIRE(tokenizer.done());
    }
    SECTION("test 3")
    {
        std::string input = R"({"A":"Jane","B":"Roe","C":10})";

        json_tokenizer tokenizer{};
        tokenizer.update(input);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);

        while (!tokenizer.done())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << tokenizer.get_string_view() << "\n";
                    break;
                case generic_token_kind::uint64_value:
                    std::cout << tokenizer.get_uint64_value() << "\n";
                    break;
                case generic_token_kind::begin_map:
                    std::cout << "begin_map\n";
                    break;
                case generic_token_kind::end_map:
                    std::cout << "end_map\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
    }
    SECTION("test 4")
    {
        std::string input = "[1,2,3]";

        json_tokenizer tokenizer{};
        tokenizer.update(input);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);

        while (!tokenizer.done())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << tokenizer.get_string_view() << "\n";
                    break;
                case generic_token_kind::uint64_value:
                    std::cout << tokenizer.get_uint64_value() << "\n";
                    break;
                case generic_token_kind::begin_array:
                    std::cout << "begin_array\n";
                    break;
                case generic_token_kind::end_array:
                    std::cout << "end_array\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
    }

    SECTION("test 5")
    {
        std::string data = R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
        )";

        json_tokenizer tokenizer{};
        tokenizer.update(data);
        REQUIRE(json_errc{} == tokenizer.try_next().ec);

        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << tokenizer.get_string_view() << "\n";
                    break;
                case generic_token_kind::uint64_value:
                    std::cout << tokenizer.get_uint64_value() << "\n";
                    break;
                case generic_token_kind::begin_array:
                    std::cout << "begin_array\n";
                    break;
                case generic_token_kind::end_array:
                    std::cout << "end_array\n";
                    break;
                case generic_token_kind::begin_map:
                    std::cout << "begin_map\n";
                    break;
                case generic_token_kind::end_map:
                    std::cout << "end_map\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
        tokenizer.try_next();
    }*/
    SECTION("test 6")
    {
        std::string data1 = R"("010)";
        std::string data2 = R"(1010)";
        std::string data3 = R"(1010)";
        std::string data4 = R"(101")";

        json_tokenizer tokenizer{};
        tokenizer.update(data1);
        REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        std::cout << "(1) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n"; 
        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout  << "(1) " << tokenizer.get_string_view() << "\n";
                    break;
                default:
                    std::cout << "(1) no token\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        }
        tokenizer.update(data2);
        REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        std::cout << "(2) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n"; 
        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << "(2) " << tokenizer.get_string_view() << "\n";
                    break;
                default:
                    std::cout << "(2) no token\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
            std::cout << "(2a) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n";
        }
        tokenizer.update(data3);
        REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        std::cout << "(3) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n"; 
        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << "(3) " << tokenizer.get_string_view() << "\n";
                    break;
                default:
                    std::cout << "no token\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        }
        tokenizer.update(data4);
        REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        std::cout << "(4) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n"; 
        while (!tokenizer.done() && !tokenizer.source_exhausted())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << "(4) " << tokenizer.get_string_view() << "\n";
                    break;
                default:
                    std::cout << "(4) no token\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next(true).ec);
        }
        std::cout << "(5) done: " << tokenizer.done() << ", source_exhausted: " << tokenizer.source_exhausted() << "\n"; 
        while (!tokenizer.done())
        {
            switch (tokenizer.token_kind())
            {
                case generic_token_kind::string_value:
                    std::cout << "(last) " << tokenizer.get_string_view() << "\n";
                    break;
                default:
                    std::cout << "(last) no token\n";
                    break;
            }
            REQUIRE(json_errc{} == tokenizer.try_next().ec);
        }
    }
}

