// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("clusterfuzz_issue 21589")
{
    std::string pathname = "input/clusterfuzz/clusterfuzz-testcase-minimized-fuzz_parse-5763671533027328";
#if 0
    SECTION("test 1")
    {
        try
        {
            json val = json::parse(is);
        }
        catch(const jsoncons::ser_error& e) 
        {
            std::cout << e.what();
        }
    }
#endif
    /*SECTION("test 2")
    {
        std::ifstream is("input/clusterfuzz/clusterfuzz-testcase-minimized-fuzz_parse-5763671533027328");
        default_json_visitor visitor;
        json_reader reader(is, visitor);

        std::error_code ec;
        reader.read(ec);
        //CHECK_FALSE(ec);
        std::cout << ec.message() << "\n";
    }*/
    /* SECTION("test 3")
    {
        std::ifstream is("input/clusterfuzz/yyy.json");
        REQUIRE(is);
        json_decoder<json> visitor;

        json_options options;
        //options.max_nesting_depth(1310);
        json_reader reader(is, visitor, options);

        std::error_code ec;
        reader.read(ec);
    }*/
    /* SECTION("test 4")
    {
        std::string s;
        json_string_encoder encoder(s);
        json_reader reader(is, encoder);

        std::error_code ec;
        reader.read(ec);
        CHECK(ec); 
        //std::cout << s << "\n";
    }*/
    /*SECTION("test 5")
    {
        json_decoder<json> visitor;
        for (size_t i = 0; i < 238; ++i)
        {
            visitor.begin_array();
        }
        for (size_t i = 0; i < 238; ++i)
        {
            visitor.end_array();
        }
        std::cout << pretty_print(visitor.get_result()) << "\n";
    }*/
    SECTION("test 6")
    {
        json_decoder<json> visitor;
        for (size_t i = 0; i < 4000; ++i)
        {
            visitor.begin_array();
        }
        for (size_t i = 0; i < 4000; ++i)
        {
            visitor.end_array();
        }
        std::cout << pretty_print(visitor.get_result()) << "\n";
    }
}

