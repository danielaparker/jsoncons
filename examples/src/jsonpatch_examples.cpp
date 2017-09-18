// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

void jsonpatch_add_add()
{
    // Apply a JSON Patch

    json doc = R"(
        { "foo": "bar"}
    )"_json;

    json doc2 = doc;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] }
        ]
    )"_json;

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(doc,patch);

    std::cout << "(1)\n" << pretty_print(doc) << std::endl;

    // Create a JSON Patch

    auto patch2 = jsonpatch::diff(doc2,doc);

    std::cout << "(2)\n" << pretty_print(patch2) << std::endl;

    std::tie(ec,path) = jsonpatch::patch(doc2,patch2);

    std::cout << "(3)\n" << pretty_print(doc2) << std::endl;
}

void jsonpatch_add_add_add_failed()
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] },
            { "op": "add", "path": "/baz/bat", "value": "qux" } // nonexistent target
        ]
    )"_json;

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(target,patch);

    std::cout << "(1) " << std::error_code(ec).message() << std::endl;
    std::cout << "(2) " << path << std::endl;
    std::cout << "(3) " << target << std::endl;
}

void create_a_json_patch()
{
    json source = R"(
        {"/": 9, "foo": "bar"}
    )"_json;

    json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    auto patch = jsonpatch::diff(source, target);

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(source,patch);

    std::cout << "(1)\n" << pretty_print(patch) << std::endl;
    std::cout << "(2)\n" << pretty_print(source) << std::endl;
}

void jsonpatch_examples()
{
    std::cout << "\njsonpatch examples\n\n";
    jsonpatch_add_add_add_failed();
    create_a_json_patch();
    jsonpatch_add_add();
    std::cout << std::endl;
}

