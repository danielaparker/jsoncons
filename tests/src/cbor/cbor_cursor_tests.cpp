// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("cbor_cursor reputon test")
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
    cbor::encode_cbor(j, data);

    SECTION("test 1")
    {
        cbor::cbor_bytes_buf_cursor cursor(data);
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

TEST_CASE("cbor_cursor playing around test")
{
    std::vector<uint8_t> data = {0x82,0x83,0x63,0x66,0x6f,0x6f,0x44,0x50,0x75,0x73,0x73,0xc3,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0x63,0x62,0x61,0x72,0xd6,0x44,0x50,0x75,0x73,0x73,0xc4,0x82,0x21,0x19,0x6a,0xb3};

    SECTION("test 1")
    {
        cbor::cbor_bytes_buf_cursor cursor(data);
        for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            switch (event.event_type())
            {
                case staj_event_type::begin_array:
                    std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::end_array:
                    std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::begin_object:
                    std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::end_object:
                    std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::name:
                    // Or std::string_view, if supported
                    std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::string_value:
                    // Or std::string_view, if supported
                    std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::byte_string_value:
                    std::cout << event.event_type() << ": " << event.get<jsoncons::byte_string_view>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::null_value:
                    std::cout << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::bool_value:
                    std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::int64_value:
                    std::cout << event.event_type() << ": " << event.get<int64_t>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::uint64_value:
                    std::cout << event.event_type() << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                    break;
                case staj_event_type::double_value:
                    std::cout << event.event_type() << ": "  << event.get<double>() << " " << "(" << event.tag() << ")\n";
                    break;
                default:
                    std::cout << "Unhandled event type " << event.event_type() << " " << "(" << event.tag() << ")\n";
                    break;
            }
        }
    }
}

