// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_stream_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("array_stream_reader test")
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    json_stream_reader reader(is);
    basic_array_stream_reader<char> array_reader(reader);

    while (!array_reader.done())
    {
        std::cout << array_reader.current() << "\n";
        array_reader.next();
    }
}

TEST_CASE("object_stream_reader test")
{
    std::string s = R"(
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        }
    )";

    std::istringstream is(s);

    json_stream_reader reader(is);
    basic_object_stream_reader<char> object_reader(reader);

    while (!object_reader.done())
    {
        std::cout << object_reader.current().key() << "\n";
        std::cout << object_reader.current().value() << "\n";
        object_reader.next();
    }
}





