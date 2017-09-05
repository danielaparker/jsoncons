// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

void jsonpatch_add_add()
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] }
        ]
    )"_json;

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(target,patch);

    std::cout << pretty_print(target) << std::endl;
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

void jsonpatch_examples()
{
    std::cout << "\njsonpatch examples\n\n";
    jsonpatch_add_add();
    jsonpatch_add_add_add_failed();
    std::cout << std::endl;
}

