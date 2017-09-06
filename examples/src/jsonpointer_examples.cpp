// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

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

    std::tie(result,ec) = jsonpointer::select(example, "");
    std::cout << "(1) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/foo");
    std::cout << "(2) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/foo/0");
    std::cout << "(3) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/");
    std::cout << "(4) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/a~1b");
    std::cout << "(5) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/c%d");
    std::cout << "(6) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/e^f");
    std::cout << "(7) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/g|h");
    std::cout << "(8) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/i\\j");
    std::cout << "(9) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/k\"l");
    std::cout << "(10) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/ ");
    std::cout << "(11) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/m~0n");
    std::cout << "(12) " << result << std::endl;
}

void jsonpointer_select_author()
{
    json root = json::parse(R"(
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
    std::tie(result,ec) = jsonpointer::select(root, "/1/author");

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

    auto ec = jsonpointer::assign(target, "/baz", json("boo"));
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

    auto ec = jsonpointer::assign(target, "/foo/1", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << pretty_print(target) << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
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
    std::cout << std::endl;
}

