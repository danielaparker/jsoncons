// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

void jsonpointer_select_RFC6901()
{
    // Example from RFC 6901
    const json example = json::parse(R"(
       {
          "foo": ["bar", "baz"],
          "": 0,
          "a/b": 1,
          "c%d": 2,
          "e^f": 3,
          "g|h": 4,
          "i\\j": 5,
          "k\"l": 6,
          " ": 7,
          "m~n": 8
       }
    )");

    try
    {
        const json& result1 = jsonpointer::get(example, "");
        std::cout << "(1) " << result1 << std::endl;
        const json& result2 = jsonpointer::get(example, "/foo");
        std::cout << "(2) " << result2 << std::endl;
        const json& result3 = jsonpointer::get(example, "/foo/0");
        std::cout << "(3) " << result3 << std::endl;
        const json& result4 = jsonpointer::get(example, "/");
        std::cout << "(4) " << result4 << std::endl;
        const json& result5 = jsonpointer::get(example, "/a~1b");
        std::cout << "(5) " << result5 << std::endl;
        const json& result6 = jsonpointer::get(example, "/c%d");
        std::cout << "(6) " << result6 << std::endl;
        const json& result7 = jsonpointer::get(example, "/e^f");
        std::cout << "(7) " << result7 << std::endl;
        const json& result8 = jsonpointer::get(example, "/g|h");
        std::cout << "(8) " << result8 << std::endl;
        const json& result9 = jsonpointer::get(example, "/i\\j");
        std::cout << "(9) " << result9 << std::endl;
        const json& result10 = jsonpointer::get(example, "/k\"l");
        std::cout << "(10) " << result10 << std::endl;
        const json& result11 = jsonpointer::get(example, "/ ");
        std::cout << "(11) " << result11 << std::endl;
        const json& result12 = jsonpointer::get(example, "/m~0n");
        std::cout << "(12) " << result12 << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void jsonpointer_contains()
{
    // Example from RFC 6901
    const json example = json::parse(R"(
       {
          "foo": ["bar", "baz"],
          "": 0,
          "a/b": 1,
          "c%d": 2,
          "e^f": 3,
          "g|h": 4,
          "i\\j": 5,
          "k\"l": 6,
          " ": 7,
          "m~n": 8
       }
    )");

    std::cout << "(1) " << jsonpointer::contains(example, "/foo/0") << std::endl;
    std::cout << "(2) " << jsonpointer::contains(example, "e^g") << std::endl;
}

void jsonpointer_select_author()
{
    json doc = json::parse(R"(
    [
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      }
    ]
    )");

    // Using exceptions to report errors
    try
    {
        json result = jsonpointer::get(doc, "/1/author");
        std::cout << "(1) " << result << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    // Using error codes to report errors
    std::error_code ec;
    const json& result = jsonpointer::get(doc, "/0/title", ec);

    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << "(2) " << result << std::endl;
    }
}

void jsonpointer_add_member_to_object()
{
    json target = json::parse(R"(
        { "foo": "bar"}
    )");

    std::error_code ec;
    jsonpointer::insert(target, "/baz", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_add_element_to_array()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::insert(target, "/foo/1", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_add_element_to_end_array()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::insert(target, "/foo/-", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_insert_name_exists()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "abc"}
    )");

    std::error_code ec;
    jsonpointer::insert(target, "/baz", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_insert_or_assign_name_exists()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "abc"}
    )");

    std::error_code ec;
    jsonpointer::insert_or_assign(target, "/baz", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_add_element_outside_range()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::insert_or_assign(target, "/foo/3", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_remove_object_member()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "qux"}
    )");

    std::error_code ec;
    jsonpointer::remove(target, "/baz", ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_remove_array_element()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::remove(target, "/foo/1", ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_replace_object_value()
{
    json target = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    std::error_code ec;
    jsonpointer::replace(target, "/baz", json("boo"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
    }
}

void jsonpointer_replace_array_value()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::replace(target, "/foo/1", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << pretty_print(target) << std::endl;
    }
}

void jsonpointer_cbor()
{
    ojson j = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> data;
    cbor::encode_cbor(j, data);

    std::error_code ec;
    cbor::cbor_view rated = jsonpointer::get(cbor::cbor_view(data), "/reputons/0/rated", ec);
    cbor::cbor_view rating = jsonpointer::get(cbor::cbor_view(data), "/reputons/0/rating", ec);

    std::cout << rated.as_string() << ", " << rating.as_double() << std::endl;
}

void jsonpointer_error_example()
{
    json doc = json::parse(R"(
    [
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      }
    ]
    )");

    try
    {
        json result = jsonpointer::get(doc, "/1/isbn");
        std::cout << "succeeded?" << std::endl;
        std::cout << result << std::endl;
    }
    catch (const jsonpointer::jsonpointer_error& e)
    {
        std::cout << "Caught jsonpointer_error with category " << e.code().category().name() 
                  << ", code " << e.code().value() 
                  << " and message \"" << e.what() << "\"" << std::endl;
    }
}

void jsonpointer_get_examples()
{
    {
        json j = json::array{"baz","foo"};
        json& item = jsonpointer::get(j,"/0");
        std::cout << "(1) " << item << std::endl;

        //std::vector<uint8_t> u;
        //cbor::encode_cbor(j,u);
        //for (auto c : u)
        //{
        //    std::cout << "0x" << std::hex << (int)c << ",";
        //}
        //std::cout << std::endl;
    }
    {
        const json j = json::array{"baz","foo"};
        const json& item = jsonpointer::get(j,"/1");
        std::cout << "(2) " << item << std::endl;
    }
    {
        std::vector<uint8_t> b = {0x82,0x63,0x62,0x61,0x7a,0x63,0x66,0x6f,0x6f};
        
        cbor::cbor_view bv = b;
        cbor::cbor_view item = jsonpointer::get(bv,"/0");
        std::cout << "(3) " << item << std::endl;
    }
    {
        json j = json::array{"baz","foo"};

        std::error_code ec;
        json& item = jsonpointer::get(j,"/1",ec);
        std::cout << "(4) " << item << std::endl;
    }
    {
        const json j = json::array{"baz","foo"};

        std::error_code ec;
        const json& item = jsonpointer::get(j,"/0",ec);
        std::cout << "(5) " << item << std::endl;
    }
    {
        std::vector<uint8_t> b = {0x82,0x63,0x62,0x61,0x7a,0x63,0x66,0x6f,0x6f};
        cbor::cbor_view bv = b;

        std::error_code ec;
        cbor::cbor_view item = jsonpointer::get(bv,"/1",ec);
        std::cout << "(6) " << item << std::endl;
    }

}

void jsonpointer_examples()
{
    std::cout << "\njsonpointer examples\n\n";
    jsonpointer_select_RFC6901();
    jsonpointer_add_member_to_object();
    jsonpointer_add_element_to_array();
    jsonpointer_add_element_to_end_array();
    jsonpointer_add_element_outside_range();
    jsonpointer_remove_object_member();
    jsonpointer_remove_array_element();
    jsonpointer_replace_object_value();
    jsonpointer_replace_array_value();
    jsonpointer_contains();
    jsonpointer_error_example();
    jsonpointer_select_RFC6901();
    jsonpointer_select_author();
    jsonpointer_insert_name_exists();
    jsonpointer_insert_or_assign_name_exists();
    jsonpointer_cbor();
    jsonpointer_get_examples();
    std::cout << std::endl;
}

