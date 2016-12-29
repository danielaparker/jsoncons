// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TEXT_TRAITS_HPP
#define JSONCONS_JSON_TEXT_TRAITS_HPP

#include <locale>
#include <string>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <cstdint> 
#include <iostream>
#include <vector>
#include <jsoncons/jsoncons.hpp>
#include <jsoncons/json_error_category.hpp>

namespace jsoncons {

inline
static bool is_continuation_byte(unsigned char ch)
{
    return (ch & 0xC0) == 0x80;
}

const uint16_t min_lead_surrogate = 0xD800;
const uint16_t max_lead_surrogate = 0xDBFF;
const uint16_t min_trail_surrogate = 0xDC00;
const uint16_t max_trail_surrogate = 0xDFFF;

inline bool is_leading_surrogate(uint16_t c)
{
    return c >= min_lead_surrogate && c <= max_lead_surrogate;
}

inline bool is_trailing_surrogate(uint16_t c)
{
    return c >= min_trail_surrogate && c <= max_trail_surrogate;
}

// json_text_traits

template <class CharT, class Enable=void>
struct json_text_traits
{
};

template <class CharT>
struct Json_text_traits_
{

    static bool is_control_character(CharT c)
    {
        uint32_t u(c >= 0 ? c : 256 + c);
        return u <= 0x1F || u == 0x7f;
    }

    static CharT to_hex_character(unsigned char c)
    {
        JSONCONS_ASSERT(c <= 0xF);

        return (c < 10) ? ('0' + c) : ('A' - 10 + c);
    }

    static bool is_non_ascii_codepoint(uint32_t cp)
    {
        return cp >= 0x80;
    }

    static std::pair<const CharT*,size_t> char_sequence_at(const CharT* it, 
                                                           const CharT* end,
                                                           size_t index)
    {
        const CharT* p = it;
        size_t count = 0;

        while (p < end && count < index)
        {
            size_t length = json_text_traits<CharT>::codepoint_length(p,end);
            p += length;
            ++count;
        }
        size_t len = json_text_traits<CharT>::codepoint_length(p,end);
        return (count == index) ? std::make_pair(p,len) : std::make_pair(it,static_cast<size_t>(0));
    }

    static size_t codepoint_count(const CharT* it, 
                                  const CharT* end)
    {
        size_t count = 0;
        const CharT* p = it;
        while (p < end)
        {
            p += json_text_traits<CharT>::codepoint_length(p,end);
            ++count;
        }
        return count;
    }

    static uint32_t codepoint_at(const CharT* it, 
                                 const CharT* end,
                                 size_t index)
    {
        uint32_t cp = 0;
        const CharT* p = it;
        size_t count = 0;
        while (p < end && count <= index)
        {
            cp = json_text_traits<CharT>::char_sequence_to_codepoint(p,end,&p);
            ++count;
        }
        return cp;
    }
};

template <class CharT>
struct json_text_traits<CharT,
                        typename std::enable_if<std::is_integral<CharT>::value &&
                        sizeof(CharT)==sizeof(uint8_t)>::type> : public Json_text_traits_<CharT>
{
    static const std::pair<const CharT*,size_t>& null_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {"null",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& true_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {"true",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& false_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {"false",5};
        return value;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 3)
        {
            uint32_t bom = static_cast<uint32_t>(static_cast<uint8_t>(it[0]) |
                                                (static_cast<uint8_t>(it[1]) << 8) |
                                                (static_cast<uint8_t>(it[2]) << 16));
            if ((bom & 0xFFFFFF) == 0xBFBBEF)  
                count += 3;
        }
        return count;
    }

    static uint32_t char_sequence_to_codepoint(const CharT* it, 
                                               const CharT* end,
                                               const CharT** stop)
    {
        CharT c = *it;
        uint32_t u(c >= 0 ? c : 256 + c );
        uint32_t cp = u;
        if (u < 0x80)
        {
            *stop = it + 1;
        }
        else if ((u >> 5) == 0x6 && (end-it) > 1)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 6) & 0x7ff) + (u & 0x3f);
            *stop = it + 1;
        }
        else if ((u >> 4) == 0xe && (end-it) > 2)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 12) & 0xffff) + ((static_cast<uint32_t>(0xff & u) << 6) & 0xfff);
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
            *stop = it + 1;
        }
        else if ((u >> 3) == 0x1e && (end-it) > 3)
        {
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp = ((cp << 18) & 0x1fffff) + ((static_cast<uint32_t>(0xff & u) << 12) & 0x3ffff);
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (static_cast<uint32_t>(0xff & u) << 6) & 0xfff;
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
            *stop = it + 1;
        }
        else
        {
            *stop = it;
        }
        return cp;
    }

    static size_t codepoint_length(const CharT* it, 
                                   const CharT* end)
    {
        size_t length = 0;
        CharT c = *it;
        uint32_t u(c >= 0 ? c : 256 + c );
        if (u < 0x80)
        {
            length = 1;
        }
        else if ((u >> 5) == 0x6 && (end-it) > 1)
        {
            length = 2;
        }
        else if ((u >> 4) == 0xe && (end-it) > 2)
        {
            length = 3;
        }
        else if ((u >> 3) == 0x1e && (end-it) > 3)
        {
            length = 4;
        }
        return length;
    }

    static void append_codepoint_to_string(uint32_t cp, std::string& s)
    {
        if (cp <= 0x7f)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x7FF)
        {
            s.push_back(static_cast<CharT>(0xC0 | (0x1f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0xFFFF)
        {
            s.push_back(0xE0 | static_cast<CharT>((0xf & (cp >> 12))));
            s.push_back(0x80 | static_cast<CharT>((0x3f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>(0xF0 | (0x7 & (cp >> 18))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & (cp >> 12))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
    }

};

template <class CharT>
struct json_text_traits<CharT,
                        typename std::enable_if<std::is_integral<CharT>::value &&
                        sizeof(CharT)==sizeof(uint16_t)>::type> : public Json_text_traits_<CharT>
{

    static const std::pair<const CharT*,size_t>& null_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"null",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& true_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"true",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& false_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"false",5};
        return value;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 1)
        {
            uint32_t bom = static_cast<uint32_t>(it[0]);
            if ((bom & 0xFFFF) == 0xFFFE)      
                ++count;
            else if ((bom & 0xFFFF) == 0xFEFF)      
                ++count;
        }
        return count;
    }

    static void append_codepoint_to_string(uint32_t cp, std::wstring& s)
    {
        if (cp <= 0xFFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>((cp >> 10) + min_lead_surrogate - (0x10000 >> 10)));
            s.push_back(static_cast<CharT>((cp & 0x3ff) + min_trail_surrogate));
        }
    }

    static uint32_t char_sequence_to_codepoint(const CharT* it, const CharT* end, const CharT** stop)
    {
        uint32_t cp = (0xffff & *it);
        if ((cp >= min_lead_surrogate && cp <= max_lead_surrogate) && (end-it) > 1) // surrogate pair
        {
            uint32_t trail_surrogate = 0xffff & *(++it);
            cp = (cp << 10) + trail_surrogate + 0x10000u - (min_lead_surrogate << 10) - min_trail_surrogate;
            *stop = it + 1;
        }
        else if (end > it)
        {
            *stop = it+1;
        }
        else
        {
            *stop = it;
        }
        return cp;
    }

    static size_t codepoint_length(const CharT* it, const CharT* end)
    {
        size_t length = 1;

        uint32_t cp = (0xffff & *it);
        if ((cp >= min_lead_surrogate && cp <= max_lead_surrogate) && (end-it) > 1) // surrogate pair
        {
            length = 2;
        }
        return length;
    }
};

template <class CharT>
struct json_text_traits<CharT,
                        typename std::enable_if<std::is_integral<CharT>::value &&
                        sizeof(CharT)==sizeof(uint32_t)>::type> : public Json_text_traits_<CharT>
{

    static const std::pair<const CharT*,size_t>& null_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"null",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& true_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"true",4};
        return value;
    }

    static const std::pair<const CharT*,size_t>& false_literal() 
    {
        static const std::pair<const CharT*,size_t> value = {L"false",5};
        return value;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 1)
        {
            uint32_t bom = static_cast<uint32_t>(it[0]);
            if (bom == 0xFFFE0000)                  
                ++count;
            else if (bom == 0x0000FEFF)             
                ++count;
        }
        return count;
    }

    static void append_codepoint_to_string(uint32_t cp, std::wstring& s)
    {
        if (cp <= 0xFFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
    }

    static uint32_t char_sequence_to_codepoint(const CharT* it, const CharT*, const CharT** stop)
    {
        uint32_t cp = static_cast<uint32_t>(*it);
        *stop = it + 1;
        return cp;
    }

    static size_t codepoint_length(const CharT* it, const CharT* end)
    {
        return (end > it) ? 1 : 0;
    }
};

}
#endif
