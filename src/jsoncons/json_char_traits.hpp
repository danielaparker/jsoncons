// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSONCHARTRAITS_HPP
#define JSONCONS_JSONCHARTRAITS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <cstdlib>

namespace jsoncons {

template <class Char>
struct json_char_traits
{
};

template <>
struct json_char_traits<wchar_t>
{
};

template <>
struct json_char_traits<char>
{
    static const std::string null_literal() {return "null";};

    static const std::string true_literal() {return "true";};

    static const std::string false_literal() {return "false";};

    static unsigned int char_sequence_to_codepoint(const std::string& s, size_t& i)
    {
        char c = s[i];
        unsigned int u(c >= 0 ? c : 256 + c );
        unsigned int cp = u;
        if (u < 0x80)
        {
        }
        else if ((u >> 5) == 0x6)
        {
            c = s[++i];
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 6) & 0x7ff) + (u & 0x3f);
        }
        else if ((u >> 4) == 0xe)
        {
            c = s[++i];
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 12) & 0xffff) + ((static_cast<unsigned int>(0xff & u) << 6) & 0xfff);
            c = s[++i];
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
        }
        else if ((u >> 3) == 0x1e)
        {
            c = s[++i];
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 18) & 0x1fffff) + ((static_cast<unsigned int>(0xff & u) << 12) & 0x3ffff);                
            c = s[++i];
            u = (c >= 0 ? c : 256 + c );
            cp += (static_cast<unsigned int>(0xff & u) << 6) & 0xfff;
            c = s[++i];
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

}
#endif
