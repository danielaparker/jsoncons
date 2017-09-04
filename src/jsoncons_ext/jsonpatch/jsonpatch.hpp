// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPATCH_HPP
#define JSONCONS_JSONPOINTER_JSONPATCH_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsoncons { namespace jsonpatch {

template <class Json>
void patch(Json& target, const Json& patch)
{
    typedef typename Json::string_type string_type;
    typedef typename Json::string_view_type string_view_type;

    const string_type test_op = string_type({'t','e','s','t'}); 
    const string_type add_op = string_type({'a','d','d'}); 
    const string_type remove_op = string_type({'r','e','m','o','v','e'}); 
    const string_type replace_op = string_type({'r','e','p','l','a','c','e'}); 
    const string_type move_op = string_type({'m','o','v','e'}); 
    const string_type copy_op = string_type({'c','o','p','y'}); 

    // Validate

    bool fail = false;
    for (const auto& operation : patch.array_range())
    {
        const string_view_type op = operation.at("op").as_string_view();
        const string_view_type path = operation.at("path").as_string_view();

        if (op == test_op)
        {
            json val;
            jsonpointer::jsonpointer_errc ec;
            std::tie(val,ec) = jsonpointer::try_select(target,path);
        }
        else if (op == add_op)
        {
        }
        else if (op == remove_op)
        {
        }
        else if (op == replace_op)
        {
        }
        else if (op == move_op)
        {
        }
        else if (op == copy_op)
        {
        }
    }
}

}}

#endif
