// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;
using namespace jsoncons::literals;

BOOST_AUTO_TEST_SUITE(json_line_split_tests)

BOOST_AUTO_TEST_CASE(test_serialization_1)
{
    json val = json::parse(R"(
    {
        "header" : {"properties": {}},
        "data":
        {
            "tags" : [],
            "id" : [1,2,3],
            "item": [[1,2,3]]    
        }
    }
)");

    std::string expected1 = R"({
    "data": {
        "id": [1,2,3],
        "item": [
            [1,2,3]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os1;
    os1 << pretty_print(val);
    BOOST_CHECK_EQUAL(expected1,os1.str());

    json_serializing_options options2 ;
    options2 .array_array_split_lines(line_split_kind::same_line);
    std::string expected2 = R"({
    "data": {
        "id": [1,2,3],
        "item": [[1,2,3]],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os2;
    os2 << pretty_print(val,options2 );
    BOOST_CHECK_EQUAL(expected2,os2.str());

    json_serializing_options options3;
    options3.array_array_split_lines(line_split_kind::new_line);
    std::string expected3 = R"({
    "data": {
        "id": [1,2,3],
        "item": [
            [1,2,3]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os3;
    os3 << pretty_print(val,options3);
    BOOST_CHECK_EQUAL(expected3,os3.str());

    json_serializing_options options4;
    options4.array_array_split_lines(line_split_kind::multi_line);
    std::string expected4 = R"({
    "data": {
        "id": [1,2,3],
        "item": [
            [
                1,
                2,
                3
            ]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os4;
    os4 << pretty_print(val,options4);
    BOOST_CHECK_EQUAL(expected4,os4.str());

    json_serializing_options options5;
    options5.object_array_split_lines(line_split_kind::same_line);
    std::string expected5 = R"({
    "data": {
        "id": [1,2,3],
        "item": [
            [1,2,3]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os5;
    os5 << pretty_print(val,options5);
    BOOST_CHECK_EQUAL(expected5,os5.str());

    json_serializing_options options6;
    options6.object_array_split_lines(line_split_kind::new_line);
    std::string expected6 = R"({
    "data": {
        "id": [
            1,2,3
        ],
        "item": [
            [1,2,3]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os6;
    os6 << pretty_print(val,options6);
    BOOST_CHECK_EQUAL(expected6,os6.str());

    json_serializing_options options7;
    options7.object_array_split_lines(line_split_kind::multi_line);
    std::string expected7 = R"({
    "data": {
        "id": [
            1,
            2,
            3
        ],
        "item": [
            [1,2,3]
        ],
        "tags": []
    },
    "header": {
        "properties": {}
    }
})";
    std::ostringstream os7;
    os7 << pretty_print(val,options7);
    BOOST_CHECK_EQUAL(expected7,os7.str());
}

// array_array_split_lines_(line_split_kind::new_line)

BOOST_AUTO_TEST_CASE(test_array_of_array_of_string_string_array)
{
    json j = R"(
[
    ["NY","LON",
        ["TOR","LON"]
    ]
]
    )"_json;

    std::cout << pretty_print(j) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()


