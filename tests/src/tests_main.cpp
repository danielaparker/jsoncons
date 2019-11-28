#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>
#include <string>
#include <iostream>
 
TEST_CASE()
{
    constexpr int n = 1;
    constexpr int m = 10;
    std::string s = "abcd";

    if constexpr (n < m || s.empty())
    {
        std::cout << "ok\n";
    }
}

