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

    json result;
    jsonpointer::jsonpointer_errc ec;

    std::tie(result,ec) = jsonpointer::get(example, "");
    std::cout << "(1) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/foo");
    std::cout << "(2) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/foo/0");
    std::cout << "(3) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/");
    std::cout << "(4) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/a~1b");
    std::cout << "(5) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/c%d");
    std::cout << "(6) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/e^f");
    std::cout << "(7) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/g|h");
    std::cout << "(8) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/i\\j");
    std::cout << "(9) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/k\"l");
    std::cout << "(10) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/ ");
    std::cout << "(11) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/m~0n");
    std::cout << "(12) " << result << std::endl;
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

    json result;
    jsonpointer::jsonpointer_errc ec;
    std::tie(result,ec) = jsonpointer::get(doc, "/1/author");

    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << result << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_add_member_to_object()
{
    json target = json::parse(R"(
        { "foo": "bar"}
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/baz", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_add_element_to_array()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/foo/1", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_add_element_to_end_array()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/foo/-", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_add_value_name_exists()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "abc"}
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/baz", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_add_element_outside_range()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/foo/3", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_remove_object_member()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "qux"}
    )");

    auto ec = jsonpointer::erase(target, "/baz");
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_remove_array_element()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    auto ec = jsonpointer::erase(target, "/foo/1");
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
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

    auto ec = jsonpointer::replace(target, "/baz", json("boo"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_replace_array_value()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::replace(target, "/foo/1", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << pretty_print(target) << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_cbor()
{
    ojson j1 = ojson::parse(R"(
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

    auto buffer = cbor::encode_cbor(j1);

    cbor::cbor_view b1(buffer); 

    cbor::cbor_view b2;
    jsonpointer::jsonpointer_errc ec;

    std::tie(b2,ec) = jsonpointer::get(b1,"/reputons/0/rated");

    ojson j2 = cbor::decode_cbor<ojson>(b2);

    std::cout << j2 << std::endl;
}

void jsonpointer_examples()
{
    std::cout << "\njsonpointer examples\n\n";
    jsonpointer_select_RFC6901();
    jsonpointer_select_author();
    jsonpointer_add_member_to_object();
    jsonpointer_add_element_to_array();
    jsonpointer_add_element_to_end_array();
    jsonpointer_add_value_name_exists();
    jsonpointer_add_element_outside_range();
    jsonpointer_remove_object_member();
    jsonpointer_remove_array_element();
    jsonpointer_replace_object_value();
    jsonpointer_replace_array_value();
    jsonpointer_contains();
    jsonpointer_cbor();
    std::cout << std::endl;
}

