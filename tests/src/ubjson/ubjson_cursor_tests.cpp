// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/ubjson/ubjson_cursor.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("ubjson_cursor reputon test")
{
    ojson j = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> data;
    ubjson::encode_ubjson(j, data);

    SECTION("test 1")
    {
        ubjson::ubjson_bytes_cursor cursor(data);
        for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            switch (event.event_type())
            {
                case staj_event_type::begin_array:
                    std::cout << "begin_array\n";
                    break;
                case staj_event_type::end_array:
                    std::cout << "end_array\n";
                    break;
                case staj_event_type::begin_object:
                    std::cout << "begin_object\n";
                    break;
                case staj_event_type::end_object:
                    std::cout << "end_object\n";
                    break;
                case staj_event_type::name:
                    // Or std::string_view, if supported
                    std::cout << "name: " << event.get<jsoncons::string_view>() << "\n";
                    break;
                case staj_event_type::string_value:
                    // Or std::string_view, if supported
                    std::cout << "string_value: " << event.get<jsoncons::string_view>() << "\n";
                    break;
                case staj_event_type::null_value:
                    std::cout << "null_value: " << "\n";
                    break;
                case staj_event_type::bool_value:
                    std::cout << "bool_value: " << std::boolalpha << event.get<bool>() << "\n";
                    break;
                case staj_event_type::int64_value:
                    std::cout << "int64_value: " << event.get<int64_t>() << "\n";
                    break;
                case staj_event_type::uint64_value:
                    std::cout << "uint64_value: " << event.get<uint64_t>() << "\n";
                    break;
                case staj_event_type::double_value:
                    std::cout << "double_value: " << event.get<double>() << "\n";
                    break;
                default:
                    std::cout << "Unhandled event type\n";
                    break;
            }
        }
    }
}

