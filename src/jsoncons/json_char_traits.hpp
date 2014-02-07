// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_CHAR_TRAITS_HPP
#define JSONCONS_JSON_CHAR_TRAITS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <cwchar>

namespace jsoncons {

template <class Char>
struct json_char_traits
{
};

template <>
struct json_char_traits<wchar_t>
{
    static size_t cstring_len(const wchar_t* s)
    {
        return std::wcslen(s);
    }

    static const std::wstring null_literal() {return L"null";};

    static const std::wstring true_literal() {return L"true";};

    static const std::wstring false_literal() {return L"false";};
};

template <>
struct json_char_traits<char>
{
    static size_t cstring_len(const char* s)
    {
        return std::strlen(s);
    }

    static const std::string null_literal() {return "null";};

    static const std::string true_literal() {return "true";};

    static const std::string false_literal() {return "false";};

    static unsigned int char_sequence_to_codepoint(std::string::const_iterator it, std::string::const_iterator end)
    {
        char c = *it;
        unsigned int u(c >= 0 ? c : 256 + c );
        unsigned int cp = u;
        if (u < 0x80)
        {
        }
        else if ((u >> 5) == 0x6)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 6) & 0x7ff) + (u & 0x3f);
        }
        else if ((u >> 4) == 0xe)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 12) & 0xffff) + ((static_cast<unsigned int>(0xff & u) << 6) & 0xfff);
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
        }
        else if ((u >> 3) == 0x1e)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 18) & 0x1fffff) + ((static_cast<unsigned int>(0xff & u) << 12) & 0x3ffff);
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (static_cast<unsigned int>(0xff & u) << 6) & 0xfff;
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
        }
        else
        {
        }
        return cp;
    }

    static void append_codepoint_to_string(unsigned int cp, std::string& s)
    {
        if (cp <= 0x7f)
        {
            s.push_back(static_cast<char>(cp));
        }
        else if (cp <= 0x7FF)
        {
            s.push_back(static_cast<char>(0xC0 | (0x1f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0xFFFF)
        {
            s.push_back(0xE0 | static_cast<char>((0xf & (cp >> 12))));
            s.push_back(0x80 | static_cast<char>((0x3f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<char>(0xF0 | (0x7 & (cp >> 18))));
            s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 12))));
            s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
    }

};

inline
bool is_control_character(unsigned int c)
{
    return c <= 0x1F;
}

}
#endif
