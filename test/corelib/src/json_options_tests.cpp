// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test json_options max_nesting_depth")
{
    std::string str = R"(
{
    "foo" : [1,2,3],
    "bar" : [4,5,{"f":6}]
}
    )";

    SECTION("success")
    {
        json_options options = json_options{}
            .max_nesting_depth(3);
        REQUIRE_NOTHROW(json::parse(str, options));
    }

    SECTION("fail")
    {
        json_options options = json_options{}
            .max_nesting_depth(2);
        REQUIRE_THROWS(json::parse(str, options));
    }
}

TEST_CASE("json_options allow_trailing_comma test")
{
    SECTION("object with trailing comma")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse("[1,2,3]");

        json val = json::parse("[1,2,3,]", options);

        CHECK(expected == val);
    }

    SECTION("array with trailing comma")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse(R"(
    {
        "first" : 1,
        "second" : 2
    }
    )", options);

        json val = json::parse(R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )", options);

        CHECK(expected == val);
    }
}

TEST_CASE("json_options allow_comments test")
{
    SECTION("allow")
    {
        auto options = json_options{}
            .allow_comments(true);

        json expected = json::parse("[1,2]");

        json val = json::parse("[1,2/*,3*/]", options);

        CHECK(expected == val);
    }

    SECTION("don't allow")
    {
        auto options = json_options{}
            .allow_comments(false);

        REQUIRE_THROWS(json::parse("[1,2/*,3*/]", options));
    }
}

TEST_CASE("test_default_nan_replacement")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    std::string buffer;
    j.dump(buffer);
    std::string expected = R"({"field1":null,"field2":null,"field3":null})";

    CHECK(expected == buffer);
}

TEST_CASE("test inf_to_num")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    SECTION("inf_to_num")
    {
        auto options = json_options{}
            .inf_to_num("1e9999");

        std::string buffer;
        j.dump(buffer, options);
        std::string expected = R"({"field1":null,"field2":1e9999,"field3":-1e9999})";

        CHECK(expected == buffer);
    }
}

TEST_CASE("object: nan_to_str, inf_to_str, neginf_to_str test")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    SECTION("pretty_print nan_to_str, inf_to_str, neginf_to_str")
    {
        auto options = json_options{}
            .nan_to_str("NaN")
            .inf_to_str("Inf")
            .neginf_to_str("NegInf")
            .root_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << buffer << "\n";
        std::string expected = R"({"field1": "NaN", "field2": "Inf", "field3": "NegInf"})";
        CHECK(expected == buffer);
    }

    SECTION("print nan_to_str, inf_to_str, neginf_to_str")
    {
        auto options = json_options{}
            .nan_to_str("NaN")
            .inf_to_str("Inf")
            .inf_to_str("NegInf");

        std::string buffer;
        j.dump(buffer, options);

        //std::cout << buffer << "\n";
        std::string expected = R"({"field1":"NaN","field2":"NegInf","field3":"-NegInf"})";
        CHECK(expected == buffer);
    }
}

TEST_CASE("array: nan_to_str, inf_to_str, neginf_to_str test")
{
    json j(json_array_arg);
    j.push_back(std::sqrt(-1.0));
    j.push_back(1.79e308 * 1000);
    j.push_back(-1.79e308 * 1000);

    SECTION("pretty_print nan_to_str, inf_to_str, neginf_to_str")
    {
        auto options = json_options{}
            .nan_to_str("NaN")
            .inf_to_str("Inf")
            .neginf_to_str("NegInf")
            .root_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << buffer << "\n";
        std::string expected = R"(["NaN", "Inf", "NegInf"])";

        CHECK(expected == buffer);
    }

    SECTION("print nan_to_str, inf_to_str, neginf_to_str")
    {
        auto options = json_options{}
            .nan_to_str("NaN")
            .inf_to_str("Inf")
            .inf_to_str("NegInf");

        std::string buffer;
        j.dump(buffer, options);

        //std::cout << buffer << "\n";
        std::string expected = R"(["NaN","NegInf","-NegInf"])";

        CHECK(expected == buffer);
    }
}

TEST_CASE("test_read_write_read_nan_replacement")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    auto options = json_options{}
        .nan_to_str("MyNaN")
        .inf_to_str("MyInf");

    std::string buffer;
    j.dump_pretty(buffer, options);

    //std::cout << buffer << "\n\n";
    json j2 = json::parse(buffer,options);

    json expected;
    expected["field1"] = std::nan("");
    expected["field2"] = std::numeric_limits<double>::infinity();
    expected["field3"] = -std::numeric_limits<double>::infinity();

    std::string output1;
    std::string output2;
    j.dump(output1, options);
    expected.dump(output2, options);

    CHECK(output1 == output2);
    CHECK(expected.to_string() == j.to_string());
}

TEST_CASE("object_array empty array")
{
    std::string s = R"(
{
    "foo": []
}
    )";

    SECTION("same_line")
    {
        json j = json::parse(s);

        auto options = json_options{}
            .object_array_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        std::string expected = R"({
    "foo": []
})";
        CHECK(expected == buffer);
    }

    SECTION("new_line")
    {
        json j = json::parse(s);

        auto options = json_options{}
            .object_array_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        std::string expected = R"({
    "foo": []
})";
        CHECK(expected == buffer);
    }

    SECTION("multi_line")
    {
        json j = json::parse(s);

        auto options = json_options{}
            .object_array_line_splits(line_split_kind::multi_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        std::string expected = R"({
    "foo": []
})";
        CHECK(expected == buffer);
    }

}

TEST_CASE("object_array with/without line_length_limit")
{
    std::string s = R"(
{
    "foo": ["bar", "baz", [1, 2, 3]],
    "qux": [1, 2, 3, null, 123, 45.3, 342334, 234]
}
    )";

    SECTION("same_line")
    {
    std::string expected = R"({
    "foo": ["bar","baz",[1,2,3]],
    "qux": [1,2,3,null,123,45.3,342334,234]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(120)
            .spaces_around_comma(spaces_option::no_spaces)
            .object_array_line_splits(line_split_kind::same_line)
            .array_array_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        CHECK(expected == buffer);
    }

    SECTION("new_line")
    {
    std::string expected = R"({
    "foo": [
        "bar","baz",
        [
            1,2,3
        ]
    ],
    "qux": [
        1,2,3,null,123,45.3,342334,234
    ]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(120)
            .spaces_around_comma(spaces_option::no_spaces)
            .array_array_line_splits(line_split_kind::new_line)
            .object_array_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }

    SECTION("multi_line")
    {
    std::string expected = R"({
    "foo": [
        "bar",
        "baz",
        [1,2,3]
    ],
    "qux": [
        1,
        2,
        3,
        null,
        123,
        45.3,
        342334,
        234
    ]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .spaces_around_comma(spaces_option::no_spaces)
            .array_array_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }

    SECTION("same_line with line length limit")
    {
    std::string expected = R"({
    "foo": ["bar","baz",[
            1,2,3
        ]],
    "qux": [1,2,3,null,
        123,45.3,342334,
        234
    ]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(20)
            .spaces_around_comma(spaces_option::no_spaces)
            .object_array_line_splits(line_split_kind::same_line)
            .array_array_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }

    SECTION("new_line with line length limit") // Revisit 234
    {
    std::string expected = R"({
    "foo": [
        "bar","baz",
        [
            1,2,3
        ]
    ],
    "qux": [
        1,2,3,null,123,
        45.3,342334,234
    ]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(20)
            .spaces_around_comma(spaces_option::no_spaces)
            .object_array_line_splits(line_split_kind::new_line)
            .array_array_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }
}

TEST_CASE("json_options line_indent")
{
    SECTION("array with line_indent_kind::same_line")
    {
        std::string j_str = R"(["1", "2", 3, 4])";
        jsoncons::json j_arr = jsoncons::json::parse(j_str);
        auto options = json_options{}
            .spaces_around_comma(jsoncons::spaces_option::space_after)
            .root_line_splits(jsoncons::line_split_kind::same_line);
        std::string buffer;
        jsoncons::encode_json_pretty(j_arr, buffer, options);

        CHECK(j_str == buffer);
    }
    SECTION("array with line_indent_kind::same_line")
    {
        std::string j_str = R"(["1", ["2", 3, 4]])";
        jsoncons::json j_arr = jsoncons::json::parse(j_str);
        auto options = json_options{}
            .spaces_around_comma(jsoncons::spaces_option::space_after)
            .root_line_splits(jsoncons::line_split_kind::same_line);
        std::string buffer;
        jsoncons::encode_json_pretty(j_arr, buffer, options);

        CHECK(j_str == buffer);
    }
}

TEST_CASE("json_options escape_solidus")
{
    SECTION("escape_solidus")
    {
        std::string j_str = R"(["1", "/2", 3, 4])";
        jsoncons::json j_arr = jsoncons::json::parse(j_str);
        auto options = json_options{}
            .escape_solidus(true);

        std::string expected = R"(["1","\/2",3,4])";
        std::string buffer;
        jsoncons::encode_json(j_arr, buffer, options);
        //std::cout << buffer << "\n";

        CHECK(expected == buffer);
    }
}

TEST_CASE("array_object with/without line_length_limit")
{
    std::string s = R"(
[
   {
       "author": "Graham Greene",
       "title": "The Comedians"
   },
   {
       "author": "Koji Suzuki",
       "title": "ring"
   },
   {
       "author": "Haruki Murakami",
       "title": "A Wild Sheep Chase"
   }
]
    )";
    SECTION("same_line")
    {
    std::string expected = R"([
    {"author": "Graham Greene","title": "The Comedians"},
    {"author": "Koji Suzuki","title": "ring"},
    {"author": "Haruki Murakami","title": "A Wild Sheep Chase"}
])";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(120)
            .spaces_around_comma(spaces_option::no_spaces)
            .array_object_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }

    SECTION("new_line")
    {
    std::string expected = R"([
    {"author": "Graham Greene","title": "The Comedians"},
    {"author": "Koji Suzuki","title": "ring"},
    {"author": "Haruki Murakami","title": "A Wild Sheep Chase"}
])";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(120)
            .spaces_around_comma(spaces_option::no_spaces)
            .array_object_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }

    SECTION("multi_line (default)")
    {
    std::string expected = R"([
    {
        "author": "Graham Greene",
        "title": "The Comedians"
    },
    {
        "author": "Koji Suzuki",
        "title": "ring"
    },
    {
        "author": "Haruki Murakami",
        "title": "A Wild Sheep Chase"
    }
])";

        json j = json::parse(s);

        auto options = json_options{}
            .spaces_around_comma(spaces_option::no_spaces);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }
    SECTION("same_line with line length limit")
    {
    std::string expected = R"([
    {"author": "Graham Greene",
     "title": "The Comedians"},
    {"author": "Koji Suzuki",
     "title": "ring"},
    {"author": "Haruki Murakami",
     "title": "A Wild Sheep Chase"}
])";

        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(20)
            .spaces_around_comma(spaces_option::no_spaces)
            .array_object_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);

        //std::cout << pretty_print(j, options) << "\n";
        CHECK(expected == buffer);
    }
    SECTION("new_line with line length limit")
    {
    std::string expected = R"([
    {"author": "Graham Greene",
     "title": "The Comedians"},
    {"author": "Koji Suzuki",
     "title": "ring"},
    {"author": "Haruki Murakami",
     "title": "A Wild Sheep Chase"}
])";
        json j = json::parse(s);

        auto options = json_options{}
            .line_length_limit(20)
            .spaces_around_comma(spaces_option::no_spaces)
            .array_object_line_splits(line_split_kind::new_line);

        std::string buffer;
        j.dump_pretty(buffer, options);
        CHECK(expected == buffer);

        //std::cout << pretty_print(j, options) << "\n";
    }
}

TEST_CASE("json_options tests")
{
    SECTION("pad_inside_array_brackets")
    {
        std::string s = R"({
    "foo": [ 1, 2 ]
})";

        json j = json::parse(s);

        auto options = json_options{}
            .pad_inside_array_brackets(true)
            .object_array_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);
        CHECK(s == buffer);
    }
    SECTION("pad_inside_object_braces")
    {
        std::string s = R"([
    { "foo": 1 }
])";

        json j = json::parse(s);

        auto options = json_options{}
            .pad_inside_object_braces(true)
            .array_object_line_splits(line_split_kind::same_line);

        std::string buffer;
        j.dump_pretty(buffer, options);
        CHECK(buffer == s);
    }
    SECTION("indent with tabs")
    {
        std::string s = "[\n\t{\n\t\t\"foo\": 1\n\t}\n]";

        json j = json::parse(s);

        auto options = json_options{}
            .indent_char('\t')
            .indent_size(1);

        std::string buffer;
        j.dump_pretty(buffer, options);
        CHECK(s == buffer);
    }
}

TEST_CASE("json_options lossless_bignum")
{
    SECTION("positive bignum")
    {
        std::string str = R"({"a":123456789012345678901234567890})";
        std::string expected = R"({"a":1.2345678901234568e+29})";
		auto options = jsoncons::json_options{}
		    .lossless_bignum(false);
		auto j = jsoncons::json::parse(str, options);
        std::string buffer;
        j.dump(buffer);
        CHECK(expected == buffer);
    }
    SECTION("negative bignum")
    {
        std::string str = R"({"a":-123456789012345678901234567890})";
        std::string expected = R"({"a":-1.2345678901234568e+29})";
        auto options = jsoncons::json_options{}
            .lossless_bignum(false);
        auto j = jsoncons::json::parse(str, options);
        std::string buffer;
        j.dump(buffer);
        CHECK(expected == buffer);
    }
    SECTION("+inf")
    {
        std::string str = R"({"a":1e999})";
        std::string expected = R"({"a":null})";
        auto options = jsoncons::json_options{}
            .lossless_bignum(false);
        auto j = jsoncons::json::parse(str, options);
        CHECK(HUGE_VAL == j["a"].as<double>());
        std::string buffer;
        j.dump(buffer);
        CHECK(expected == buffer);
    }
    SECTION("-inf")
    {
        std::string str = R"({"a":-1e999})";
        std::string expected = R"({"a":null})";
        auto options = jsoncons::json_options{}
            .lossless_bignum(false);
        auto j = jsoncons::json::parse(str, options);
        CHECK(-HUGE_VAL == j["a"].as<double>());
        std::string buffer;
        j.dump(buffer);
        CHECK(expected == buffer);
    }
}

TEST_CASE("test_array_extra_comma")
{
    SECTION("with option")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse("[1,2,3]");

        json val1 = json::parse("[1,2,3,]", options);

        CHECK(expected == val1);
    }
}

TEST_CASE("test_object_extra_comma")
{
    SECTION("with option")
    {
        auto options = json_options{}
            .allow_trailing_comma(true);

        json expected = json::parse(R"(
    {
        "first" : 1,
        "second" : 2
    }
    )", options);

        json val = json::parse(R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )", options);

        CHECK(expected == val);
    }
}

