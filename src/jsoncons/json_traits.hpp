// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TRAITS_HPP
#define JSONCONS_JSON_TRAITS_HPP

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

#include <jsoncons/output_format.hpp>
#include <jsoncons/parse_error_handler.hpp>

namespace jsoncons {

template <class CharT>
struct json_traits
{
    static const bool is_object_sorted = true;

    typedef basic_output_format<CharT> output_format_type;
    typedef basic_parse_error_handler<CharT> parse_error_handler_type;
};

template <class CharT>
struct ojson_traits 
{
    static const bool is_object_sorted = false;

    typedef basic_output_format<CharT> output_format_type;
    typedef basic_parse_error_handler<CharT> parse_error_handler_type;
};

}

#endif
