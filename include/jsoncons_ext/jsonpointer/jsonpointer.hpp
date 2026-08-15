// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_JSONPOINTER_JSONPOINTER_HPP
#define JSONCONS_EXT_JSONPOINTER_JSONPOINTER_HPP

#include <string>
#include <jsoncons_ext/jsonpointer/json_pointer.hpp>

namespace jsoncons { 
namespace jsonpointer {

template <typename CharT,typename Allocator=std::allocator<CharT>>
std::basic_string<CharT,std::char_traits<CharT>,Allocator> escape(jsoncons::basic_string_view<CharT> s, const Allocator& = Allocator())
{
    std::basic_string<CharT,std::char_traits<CharT>,Allocator> result;

    for (auto c : s)
    {
        if (JSONCONS_UNLIKELY(c == '~'))
        {
            result.push_back('~');
            result.push_back('0');
        }
        else if (JSONCONS_UNLIKELY(c == '/'))
        {
            result.push_back('~');
            result.push_back('1');
        }
        else
        {
            result.push_back(c);
        }
    }
    return result;
}

template <typename CharT>
std::basic_string<CharT> escape_string(const std::basic_string<CharT>& s)
{
    std::basic_string<CharT> result;
    for (auto c : s)
    {
        switch (c)
        {
            case '~':
                result.push_back('~');
                result.push_back('0');
                break;
            case '/':
                result.push_back('~');
                result.push_back('1');
                break;
            default:
                result.push_back(c);
                break;
        }
    }
    return result;
}

} // namespace jsonpointer
} // namespace jsoncons

#endif
