// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <fstream>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

namespace {

    void search_example() 
    {
    }

    void jmespath_expression_example()
    { 
    }

} // namespace

void jmespath_examples()
{
    std::cout << "\nJMESPath examples\n\n";
    search_example();
    jmespath_expression_example();

    std::cout << "\n";
}

