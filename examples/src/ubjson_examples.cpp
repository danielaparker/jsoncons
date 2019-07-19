// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include "example_types.hpp"
#include <jsoncons/json.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <string>
#include <iomanip>
#include <cassert>

using namespace jsoncons;

void to_from_ubjson_using_basic_json()
{
    ojson j1 = ojson::parse(R"(
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

    // Encode a basic_json value to UBJSON
    std::vector<uint8_t> data;
    ubjson::encode_ubjson(j1, data);

    // Decode UBJSON to a basic_json value
    ojson j2 = ubjson::decode_ubjson<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Accessing the data items 

    const ojson& reputons = j2["reputons"];

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as<std::string>() << ", ";
        std::cout << element.at("rating").as<double>() << "\n";
    }
    std::cout << std::endl;

    // Get a UBJSON value for a nested data item with jsonpointer
    std::error_code ec;
    auto const& rated = jsonpointer::get(j2, "/reputons/0/rated", ec);
    if (!ec)
    {
        std::cout << "(3) " << rated.as_string() << "\n";
    }

    std::cout << std::endl;
}

void to_from_ubjson_using_example_type()
{
    ns::hiking_reputation val("hiking", { ns::hiking_reputon{"HikingAsylum",ns::hiking_experience::advanced,"Marilyn C",0.90} });

    // Encode a ns::hiking_reputation value to UBJSON
    std::vector<uint8_t> data;
    ubjson::encode_ubjson(val, data);

    // Decode UBJSON to a ns::hiking_reputation value
    ns::hiking_reputation val2 = ubjson::decode_ubjson<ns::hiking_reputation>(data);

    assert(val2 == val);
}

void working_with_ubjson_2()
{
    /*[[][$][d][#][i][5] // An array of 5 float32 elements.
    [29.97] // Value type is known, so type markers are omitted.
    [31.13]
    [67.0]
    [2.113]
    [23.8889]*/
// No end marker since a count was specified.

    std::vector<uint8_t> data =
    {0x5b,0x23,0x55,0x05,
    0x44,
        0x40,0x3d,0xf8,0x51,0xeb,0x85,0x1e,0xb8,
    0x44,
        0x40,0x3f,0x21,0x47,0xae,0x14,0x7a,0xe1,
    0x64,
        0x42,0x86,0x00,0x00,
    0x44,
        0x40,0x00,0xe7,0x6c,0x8b,0x43,0x95,0x81,
    0x44,
        0x40,0x37,0xe3,0x8e,0xf3,0x4d,0x6a,0x16};

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

    /* std::string s = "[29.97,31.13,67.0,2.113,23.8889]";
    auto j = decode_json<json>(s);

    std::vector<uint8_t> data;
    ubjson::encode_ubjson(j,data);
    for (auto c : data)
    {
        std::cout << "'0x" << std::hex << std::setprecision(2) << std::setw(2) 
                  << std::noshowbase << std::setfill('0') << (int)unsigned char(c) << "',";
    }
    std::cout << "\n\n"; */
}

void ubjson_examples()
{
    std::cout << "\nubjson examples\n\n";
    to_from_ubjson_using_basic_json();
    to_from_ubjson_using_example_type();
    working_with_ubjson_2();

    std::cout << std::endl;
}

