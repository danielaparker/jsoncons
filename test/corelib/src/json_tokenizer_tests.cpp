// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json_tokenizer.hpp>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("json_tokenizer constructor test")
{
    SECTION("default constructor")
    {
        json_tokenizer tokenizer{};
        std::string str = R"(
{
    "foo" : "bar"

}
)";

        jsoncons::from_json_result r;
        tokenizer.update(str.data(), str.size());
        while (!tokenizer.done())
        {
            if (!r)
            {
                std::cout << (int)r.ec << "\n";
                //tokenizer.update("}", 1);
                //break;
            }
            else
            {
                std::cout << tokenizer.event_kind() << "\n";
            }
            r = tokenizer.try_next();
        }
    }
}
