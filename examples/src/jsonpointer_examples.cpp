// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

void jsonpointer_select()
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

    try
    {
        json result = jsonpointer::select(root, "/1/author");
        std::cout << result << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_select()
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
    std::tie(result,ec) = jsonpointer::try_select(root, "/1/author");

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

    try
    {
        jsonpointer::add(target, "/baz", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_add_member_to_object()
{
    json target = json::parse(R"(
    { "foo": "bar"}
    )");

    auto ec = jsonpointer::try_add(target, "/baz", json("qux"));
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

    try
    {
        jsonpointer::add(target, "/foo/1", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_add_element_to_array()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::try_add(target, "/foo/1", json("qux"));
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

    try
    {
        jsonpointer::add(target, "/foo/-", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_add_element_to_end_array()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::try_add(target, "/foo/-", json("qux"));
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

    try
    {
        jsonpointer::add(target, "/foo/3", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_add_element_outside_range()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::try_add(target, "/foo/3", json("qux"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_remove()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    try
    {
        jsonpointer::remove(target, "/foo/1");
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_remove()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    auto ec = jsonpointer::try_remove(target, "/foo/1");
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}

void jsonpointer_replace()
{
    json target = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    try
    {
        jsonpointer::replace(target, "/baz", json("boo"));
        std::cout << pretty_print(target) << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void jsonpointer_try_replace()
{
    json target = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    auto ec = jsonpointer::try_replace(target, "/baz", json("boo"));
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
    jsonpointer_select();
    jsonpointer_try_select();
    jsonpointer_add_member_to_object();
    jsonpointer_try_add_member_to_object();
    jsonpointer_add_element_to_array();
    jsonpointer_try_add_element_to_array();
    jsonpointer_add_element_to_end_array();
    jsonpointer_try_add_element_to_end_array();
    jsonpointer_add_element_outside_range();
    jsonpointer_try_add_element_outside_range();
    jsonpointer_remove();
    jsonpointer_try_remove();
    jsonpointer_replace();
    jsonpointer_try_replace();
    std::cout << std::endl;
}

