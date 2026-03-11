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
            if (opts.contains("indent"))
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
        bool should_error = false;
        if (test.contains("options"))
        {
            const auto& opts = test["options"]; 
            if (opts.contains("delimiter"))
            {
                auto sv = opts["delimiter"].as_string_view();
                options.delimiter(sv.front());
            }
            if (opts.contains("indent"))
            {
                auto value = opts["indent"].as<int>();
                options.indent(value);
            }
            if (opts.contains("strict"))
            {
                auto value = opts["strict"].as<bool>();
                options.strict(value);
            }
        }
        auto input = test["input"].as_string_view();
        const auto& expected = test["expected"];
        if (test.contains("shouldError"))
        {
            should_error = test["shouldError"].as<bool>();
        }

        try
        {
            jsoncons::json_decoder<jsoncons::ojson> decoder;
            toon::toon_string_reader reader(input, decoder, options);
            reader.read();
            if (should_error)
            {
                std::cout << test.at("name").as_string() << "\n";
                std::cout << input << "\n";
                REQUIRE(!should_error);
            }
            if (!decoder.is_valid())
            {
                std::cout << test.at("name").as_string() << "\n";
                std::cout << input << "\n";
            }
            REQUIRE(decoder.is_valid());
            auto result = decoder.get_result();
            CHECK(expected == result);

            if (expected != result)
            {
                std::cout << "shouldError: " << should_error << "\n";
                std::cout << test.at("name").as_string() << "\n";
                std::cout << input << "\n";
                std::cout << pretty_print(result) << "\n";
            }
            CHECK(expected == result);
        }
        catch (const std::exception&)
        {
            if (!should_error)
            {
                std::cout << "Except shouldError: " << should_error << "\n";
                std::cout << test.at("name").as_string() << "\n";
                std::cout << input << "\n";
                CHECK(should_error);
            }
        }
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
        toon_decode_tests("./toon/input/decode/arrays-primitive.json");
        toon_decode_tests("./toon/input/decode/arrays-tabular.json");
        toon_decode_tests("./toon/input/decode/blank-lines.json");
        toon_decode_tests("./toon/input/decode/delimiters.json");
        toon_decode_tests("./toon/input/decode/indentation-errors.json");
        //toon_decode_tests("./toon/input/decode/numbers.json");
        //toon_decode_tests("./toon/input/decode/objects.json");
        //toon_decode_tests("./toon/input/decode/path-expansion.json");
        //toon_decode_tests("./toon/input/decode/primitives.json");
        //toon_decode_tests("./toon/input/decode/root-form.json");
        //toon_decode_tests("./toon/input/decode/validation-errors.json");
        //toon_decode_tests("./toon/input/decode/whitespace.json");
    }
}

