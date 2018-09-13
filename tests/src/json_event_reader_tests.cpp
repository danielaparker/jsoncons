// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_event_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("json event reader tests")
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

    json_event_reader event_reader(is);

    for (const auto& event : event_reader)
    {
        switch (event.event_type())
        {
            case json_event_type::name:
                std::cout << event.as<std::string>() << ": ";
                break;
            case json_event_type::string:
                std::cout << event.as<std::string>() << "\n";
                break;
            case json_event_type::integer:
            case json_event_type::uinteger:
                std::cout << event.as<std::string>() << "\n";
                break;
        }
    }
}




