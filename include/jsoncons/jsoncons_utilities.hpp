// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_UTILITIES_HPP
#define JSONCONS_JSONCONS_UTILITIES_HPP

#include <stdexcept>
#include <string>
#include <vector>
#include <ostream>
#include <cmath>
#include <algorithm> // std::find, std::min, std::reverse
#include <memory>
#include <iterator>
#include <exception>
#include <stdexcept>
#include <istream> // std::basic_istream
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/byte_string.hpp>
#include <jsoncons/json_exception.hpp>
#if !defined(JSONCONS_NO_TO_CHARS)
#include <charconv>
#endif

namespace jsoncons
{

template <class CharT>
class basic_null_istream : public std::basic_istream<CharT>
{
    class null_buffer : public std::basic_streambuf<CharT>
    {
    public:
        using typename std::basic_streambuf<CharT>::int_type;
        using typename std::basic_streambuf<CharT>::traits_type;
        int_type overflow( int_type ch = traits_type::eof() ) override
        {
            return ch;
        }
    } nb_;
public:
    basic_null_istream()
      : std::basic_istream<CharT>(&nb_)
    {
    }
};

// json_literals

namespace detail {

template <class CharT>
basic_string_view<CharT> null_literal()
{
    static const CharT chars[] = {'n','u','l', 'l'};
    return basic_string_view<CharT>(chars,sizeof(chars)/sizeof(CharT));
}

template <class CharT>
basic_string_view<CharT> true_literal()
{
    static const CharT chars[] = {'t','r','u', 'e'};
    return basic_string_view<CharT>(chars,sizeof(chars)/sizeof(CharT));
}

template <class CharT>
basic_string_view<CharT> false_literal()
{
    static const CharT chars[] = {'f','a','l', 's', 'e'};
    return basic_string_view<CharT>(chars,sizeof(chars)/sizeof(CharT));
}

}

}

#endif
