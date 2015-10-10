// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSONPATH_JSONPATH_HPP
#define JSONCONS_JSONPATH_JSONPATH_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_structures.hpp"

namespace jsoncons { namespace jsonpath {

namespace states {
    enum states_t {
        start,
        path_separator,
        string
    };
};

template<typename Char, class Alloc>
basic_json<Char,Alloc> jsonpath(const basic_json<Char, Alloc>& root, const Char* path)
{
    return jsonpath(root,path,strlen(path));
}

template<typename Char, class Alloc>
basic_json<Char,Alloc> jsonpath(const basic_json<Char, Alloc>& root, const Char* path, size_t path_length)
{
    typedef const basic_json<Char,Alloc>* cjson_ptr;

    basic_json<Char,Alloc> result(basic_json<Char,Alloc>::an_array);

    states::states_t state = jsoncons::states::start;
    std::vector<std::vector<cjson_ptr>> stack;
    std::basic_string<Char> buffer;

    for (size_t i = 0; i < path_length; ++i)
    {
        Char c = path[i];
        switch (state)
        {
        case states::start: 
            switch (c)
            {
            case '$':
                std::vector<cjson_ptr> v(1);
                v.push_back(&root);
                stack.push_back(v);
                state = states::path_separator;
                break;
            };
            break;
        case states::path_separator: 
            switch (c)
            {
            case '.':
                buffer.clear();
                state = states::string;
                break;
            };
            break;
        case states::string: 
            switch (c)
            {
            case '.':
                buffer.clear();
                state = states::string;
                break;
            default:
                buffer.push_back(c);
                break;
            };
            break;
        }
    }
    return result;
}

}}

#endif
