// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <fstream>

using namespace jsoncons;

void parse_with_comment()
{
    std::string s = R"(
    {
        // Single line comments
        /*
            Multi line comments 
        */
    }
    )";

    // Default
    {
        auto j = json::parse(s);
        std::cout << "(1) " << j << std::endl;
    }
    // Strict
    try
    {
        // until 0.170.0
        auto j1 = json::parse(s, strict_json_parsing());

        // since 0.171.0
        json_options options;
        options.err_handler(strict_json_parsing());
        auto j2 = json::parse(s, options);
    }
    catch (const ser_error& e)
    {
        std::cout << "(2) " << e.what() << std::endl;
    }
}

void parse_with_trailing_commas()
{
    std::string s = R"(
    {
        "first" : 1,
        "second" : 2,
    }
    )";

    // Default
    try
    {
        auto j = json::parse(s);
    }
    catch (const ser_error& e)
    {
        std::cout << "(1) " << e.what() << "\n\n";
    }

    // Allow trailing commas

    // until 0.170.0
    // auto j = json::parse(s, allow_trailing_commas());

    // since 0.171.0
    json_options options;
    options.err_handler(allow_trailing_commas());
    auto j = json::parse(s, options);
    std::cout << "(2) " << j << "\n\n";
}

void parse_error_example()
{
    std::string s = "[1,2,3,4,]";
    try 
    {
        json val = json::parse(s);
    } 
    catch(const ser_error& e) 
    {
        std::cout << "Caught ser_error with category " << e.code().category().name() 
                  << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}

void max_nesting_path_example()
{
    std::string s = "[[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]]";
    try
    {
        json_options options;
        options.max_nesting_depth(20);
        json::parse(s, options);
    }
    catch (const ser_error& e)
    {
         std::cout << e.what() << std::endl;
    }
}

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR)
#include <memory_resource> 

// since 0.171.0
void using_polymorphic_allocator()
{
    using pmr_json = pmr::json;

    char buffer[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool{std::data(buffer), std::size(buffer)};
    std::pmr::polymorphic_allocator<char> alloc(&pool);

    std::string json_text = R"(
    {
        "street_number" : "100",
        "street_name" : "Queen St W",
        "city" : "Toronto",
        "country" : "Canada"
    }
    )";


    try
    {
        auto doc = pmr_json::parse(combine_allocators(alloc), json_text, json_options{});
        std::cout << pretty_print(doc) << "\n\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

#endif

int main()
{
    try
    {
        std::cout << "jsoncons version: " << version() << std::endl;

        parse_error_example();

        parse_with_comment();

        max_nesting_path_example();

        using_polymorphic_allocator();

        parse_with_trailing_commas();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
