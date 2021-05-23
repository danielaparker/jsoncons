// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <unordered_set> // std::unordered_set
#include <fstream>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

TEST_CASE("jsonpath custom function test")
{
    json root;
    JSONCONS_TRY
    {
        root = json::parse(R"({ "foo": 60,"bar": 10 })");
    }
    JSONCONS_CATCH (const jsoncons::ser_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    std::vector<jsonpath::custom_function<json>> functions = {
        {"divide", // function name
         2,        // number of arguments   
         [](jsoncons::span<const jsonpath::parameter<json>> params, std::error_code& ec) -> json 
          {
            if (!(params[0].value().is_number() && params[1].value().is_number())) 
            {
                ec = jsonpath::jsonpath_errc::invalid_type; 
                return json::null();
            }
            return json(params[0].value().as<double>() / params[1].value().as<double>());}
        }
    };

    SECTION("test 1")
    {
        auto expr = jsonpath::make_expression<json>("divide(@.foo, @.bar)", functions);
        auto r = expr.evaluate(root);
        REQUIRE(!r.empty());
        CHECK(r[0] == json(6));

        //auto result = jsonpath::json_query(root,"divide($.foo, $.bar)");
        //auto expected = json::parse(R"([6])");
        //CHECK((result == expected));
    }

    SECTION("test 2")
    {
        auto r = jsonpath::json_query(root, "divide($.foo, $.bar)", jsonpath::result_options(), functions);
        REQUIRE(!r.empty());
        CHECK(r[0] == json(6));

        //auto result = jsonpath::json_query(root,"divide($.foo, $.bar)");
        //auto expected = json::parse(R"([6])");
        //CHECK((result == expected));
    }
}

