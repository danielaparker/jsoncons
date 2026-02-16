// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_URI_HPP
#define JSONCONS_UTILITY_URI_HPP

#include <string> // std::string

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 

template <typename CharT>
bool starts_with(jsoncons::basic_string_view<CharT> sv, 
    jsoncons::basic_string_view<CharT> prefix)
{
    if (JSONCONS_UNLIKELY(sv.size() < prefix.size()))
    {
        return false;
    }
    const CharT* p = sv.data();
    const CharT* q = prefix.data();
    const CharT* last = prefix.data() + prefix.size();

    while (q < last)
    {
        if (*q++ != *p++)
        {
            return false;
        }
    }
    return true;
}

template <typename CharT>
bool starts_with(const CharT* s, const CharT* prefix)
{
    return starts_with(jsoncons::basic_string_view<CharT>(s),
        jsoncons::basic_string_view<CharT>(prefix));
}

template <typename CharT>
bool ends_with(jsoncons::basic_string_view<CharT> sv, 
    jsoncons::basic_string_view<CharT> suffix)
{
    if (JSONCONS_UNLIKELY(sv.size() < suffix.size()))
    {
        return false;
    }
    const CharT* p = sv.data() + (sv.size() - suffix.size());
    const CharT* q = suffix.data();
    const CharT* last = suffix.data() + suffix.size();

    while (q < last)
    {
        if (*q++ != *p++)
        {
            return false;
        }
    }
    return true;
}

template <typename CharT>
bool ends_with(const CharT* s, const CharT* suffix)
{
    return ends_with(jsoncons::basic_string_view<CharT>(s),
        jsoncons::basic_string_view<CharT>(suffix));
}

template <typename CharT>
jsoncons::basic_string_view<CharT> strip(jsoncons::basic_string_view<CharT> sv)
{
    const CharT* first = sv.data();
    const CharT* last = first + sv.size();
    const CharT* p = first;

    while (p < last)
    {
        CharT c = *p;
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        {
            break;
        }
        ++p;
    }
    if (p == last)
    {
        return jsoncons::basic_string_view<CharT>{};
    }

    const CharT* q = last;
    do
    {
        --q;
        char c = *q;
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        {
            break;
        }
    } while (q > p);

    std::size_t size = (p - first) + ((last-q)-1);
    return jsoncons::basic_string_view<CharT>(p, sv.size() - size);
}

template <typename CharT>
jsoncons::basic_string_view<CharT> strip(const CharT* s)
{
    return strip(jsoncons::basic_string_view<CharT>{s});
}

} // namespace jsoncons

#endif // JSONCONS_UTILITY_URI_HPP
