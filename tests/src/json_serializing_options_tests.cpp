// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;
#if 0
TEST_CASE("test_default_nan_replacement")
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;

    std::ostringstream os;
    os << print(obj);
    std::string expected = R"({"field1":null,"field2":null,"field3":null})";

    CHECK(expected == os.str());
}

TEST_CASE("test_write_nan_replacement")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    json_serializing_options options;
    options.nan_replacement("null")
           .pos_inf_replacement("1e9999")
           .neg_inf_replacement("-1e9999");

    std::ostringstream os;
    os << print(j, options);
    std::string expected = R"({"field1":null,"field2":1e9999,"field3":-1e9999})";

    CHECK(expected == os.str());
}

TEST_CASE("test_read_write_read_nan_replacement")
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    json_serializing_options options;
    options.nan_replacement("\"NaN\"")
           .pos_inf_replacement("\"Inf\"")
           .neg_inf_replacement("\"-Inf\"");

    std::ostringstream os;
    os << pretty_print(j, options);

    json j2 = json::parse(os.str(),options);

    json expected;
    expected["field1"] = std::nan("");
    expected["field2"] = std::numeric_limits<double>::infinity();
    expected["field3"] = -std::numeric_limits<double>::infinity();

    CHECK(expected.to_string(options) == j.to_string(options));
    CHECK(expected.to_string() == j.to_string());
}
#endif
TEST_CASE("line_length_limit tests")
{
    std::string s = R"(
{
    "code": 105,
    "tags": ["persistent", "priority"],
    "props": {
        "finished": true,
        "logged": false
    },
    "body": [1, 76, 345, 34, 234.3, 34, null, 234, 454, 278, 3, 55, 90, 45, 45.3, 234, 234, 234, 56, 3, 25, 342334, 234, 3, 6, 6, 45, 33, 34, 433, 65, 56, 3, 454, 3, 54, 45]
}
    )";
#if 0
    SECTION("Defaults")
    {
        json j = json::parse(s);

        json_serializing_options options;
        options.line_length_limit(22)
               .spaces_around_comma(spaces_option::space_after);

        std::ostringstream os;
        os << pretty_print(j, options);

        std::cout << pretty_print(j, options) << "\n";
    }
#endif
    SECTION("object_array_split_lines(line_split_kind::new_line)")
    {
        json j = json::parse(s);

        json_serializing_options options;
        options.line_length_limit(22)
               .spaces_around_comma(spaces_option::space_after)
               .object_array_split_lines(line_split_kind::multi_line);

        std::ostringstream os;
        os << pretty_print(j, options);

        std::cout << pretty_print(j, options) << "\n";
    }

}

