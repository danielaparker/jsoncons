// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/toon/encode_toon.hpp>
#include <jsoncons_ext/toon/toon_reader.hpp>
#include <jsoncons/json.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <unordered_set> // std::unordered_set
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

void toon_encode_tests(const std::string& fpath)
{
    std::cout << "Test " << fpath << '\n';

    std::fstream is(fpath);
    if (!is)
    {
        std::cerr << "Cannot open " << fpath << "\n";
        exit(1);
    }

    auto suite = ojson::parse(is);
    const auto& tests = suite.at("tests"); 
    for (const auto& test : tests.array_range())
    {
        toon::toon_options options{};
        if (test.contains("options"))
        {
            const auto& opts = test["options"]; 
            if (opts.contains("delimiter"))
            {
                auto sv = opts["delimiter"].as_string_view();
                options.delimiter(sv.front());
            }
            else if (opts.contains("indent"))
            {
                auto value = opts["indent"].as<int>();
                options.indent(value);
            }
        }
        const auto& j = test["input"];
        auto expected = test["expected"].as_string_view();
        std::string buffer;
        toon::encode_toon(j, buffer, options);
        if (expected != buffer)
        {
            std::cout << test.at("name").as_string() << "\n";
            std::cout << pretty_print(j) << "\n";
        }
        CHECK(expected == buffer);
    }
}

void toon_decode_tests(const std::string& fpath)
{
    std::cout << "Test " << fpath << '\n';

    std::fstream is(fpath);
    if (!is)
    {
        std::cerr << "Cannot open " << fpath << "\n";
        exit(1);
    }

    auto suite = ojson::parse(is);
    const auto& tests = suite.at("tests"); 
    for (const auto& test : tests.array_range())
    {
        toon::toon_options options{};
        if (test.contains("options"))
        {
            const auto& opts = test["options"]; 
            if (opts.contains("delimiter"))
            {
                auto sv = opts["delimiter"].as_string_view();
                options.delimiter(sv.front());
            }
            else if (opts.contains("indent"))
            {
                auto value = opts["indent"].as<int>();
                options.indent(value);
            }
        }
        auto input = test["input"].as_string_view();
        const auto& expected = test["expected"];

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        toon::toon_string_reader reader(input, decoder);
        reader.read();
        REQUIRE(decoder.is_valid());
        auto result = decoder.get_result();
        CHECK(expected == result);

        if (expected != result)
        {
            std::cout << test.at("name").as_string() << "\n";
            std::cout << input << "\n";
            std::cout << pretty_print(result) << "\n";
        }
        CHECK(expected == result);
    }
}

TEST_CASE("toon-tests")
{
    /*SECTION("encode")
    {
        toon_encode_tests("./toon/input/encode/arrays-nested.json");
        toon_encode_tests("./toon/input/encode/arrays-objects.json");
        toon_encode_tests("./toon/input/encode/arrays-primitive.json");
        toon_encode_tests("./toon/input/encode/arrays-tabular.json");
        toon_encode_tests("./toon/input/encode/delimiters.json");
        //toon_encode_tests("./toon/input/encode/key-folding.json");
        toon_encode_tests("./toon/input/encode/objects.json");
        toon_encode_tests("./toon/input/encode/primitives.json");
        toon_encode_tests("./toon/input/encode/whitespace.json");
    }*/
    SECTION("decode")
    {
        toon_decode_tests("./toon/input/decode/arrays-nested.json");
    }
}

