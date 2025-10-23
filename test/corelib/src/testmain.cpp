#define CATCH_CONFIG_MAIN

#include <jsoncons/config/compiler_support.hpp>
#include <catch/catch.hpp>

#include <iostream>

TEST_CASE("configuration")
{
    //memory leak test
    //char* p = new char [10];
    //p[0] = 0;

    #if defined(__clang__) 
        std::cout << "clang" << "\n";
    #elif defined(__GNUC__)
        std::cout << "__GNUC__: " << __GNUC__ << "\n";
        #if defined(__GNUC_MINOR__)
            std::cout << "__GNUC_MINOR__" << __GNUC_MINOR__ << "\n";
        #endif
    #endif
    #if defined(JSONCONS_HAS_STD_REGEX)
        std::cout << "JSONCONS_HAS_STD_REGEX\n";
    #endif
}
