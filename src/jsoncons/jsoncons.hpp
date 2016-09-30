// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_HPP
#define JSONCONS_JSONCONS_HPP

#include <locale>
#include <string>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <cstdint> 
#include <iostream>
#include <vector>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/jsoncons_io.hpp"

namespace jsoncons {

template <size_t arg1, size_t ... argn>
struct static_max;

template <size_t arg>
struct static_max<arg>
{
    static const size_t value = arg;
};

template <size_t arg1, size_t arg2, size_t ... argn>
struct static_max<arg1,arg2,argn ...>
{
    static const size_t value = arg1 >= arg2 ? 
        static_max<arg1,argn...>::value :
        static_max<arg2,argn...>::value; 
};

// null_type

struct null_type
{
};

// json_exception

class json_exception 
{
public:
    virtual const char* what() const JSONCONS_NOEXCEPT = 0;
};

template <class Base>
class json_exception_0 : public Base, public virtual json_exception
{
public:
    json_exception_0(std::string s) JSONCONS_NOEXCEPT
        : Base(""), message_(s)
    {
    }
    ~json_exception_0() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

template <class Base>
class json_exception_1 : public Base, public virtual json_exception
{
public:
    json_exception_1(const std::string& format, const std::string& arg1) JSONCONS_NOEXCEPT
        : Base(""), format_(format), arg1_(arg1)
    {
    }
    json_exception_1(const std::string& format, const std::wstring& arg1) JSONCONS_NOEXCEPT
        : Base(""), format_(format)
    {
        char buf[255];
        size_t retval;
#if defined(JSONCONS_HAS_WCSTOMBS_S)
        wcstombs_s(&retval, buf, sizeof(buf), arg1.c_str(), arg1.size());
#else
        retval = wcstombs(buf, arg1.c_str(), sizeof(buf));
#endif
        if (retval != static_cast<std::size_t>(-1))
        {
            arg1_ = buf;
        }
    }
    ~json_exception_1() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        c99_snprintf(const_cast<char*>(message_),255, format_.c_str(),arg1_.c_str());
        return message_;
    }
private:
    std::string format_;
    std::string arg1_;
    char message_[255];
};

#define JSONCONS_STR2(x)  #x
#define JSONCONS_STR(x)  JSONCONS_STR2(x)

#define JSONCONS_THROW_EXCEPTION(Base,x) throw jsoncons::json_exception_0<Base>((x))
#define JSONCONS_THROW_EXCEPTION_1(Base,fmt,arg1) throw jsoncons::json_exception_1<Base>((fmt),(arg1))
#define JSONCONS_ASSERT(x) if (!(x)) { \
    throw jsoncons::json_exception_0<std::runtime_error>("assertion '" #x "' failed at " __FILE__ ":" \
            JSONCONS_STR(__LINE__)); }

// json_text_traits

const uint16_t min_lead_surrogate = 0xD800;
const uint16_t max_lead_surrogate = 0xDBFF;
const uint16_t min_trail_surrogate = 0xDC00;
const uint16_t max_trail_surrogate = 0xDFFF;

template <class CharT>
struct json_text_traits
{
};

template <>
struct json_text_traits<char>
{
    static const std::pair<const char*,size_t>& null_literal() 
    {
        static const std::pair<const char*,size_t> value = {"null",4};
        return value;
    }

    static const std::pair<const char*,size_t>& true_literal() 
    {
        static const std::pair<const char*,size_t> value = {"true",4};
        return value;
    }

    static const std::pair<const char*,size_t>& false_literal() 
    {
        static const std::pair<const char*,size_t> value = {"false",5};
        return value;
    }

    static size_t skip_bom(const char* it, size_t length)
    {
        size_t count = 0;
        if (length >= 3)
        {
            uint32_t bom = static_cast<uint32_t>(static_cast<unsigned char>(it[0]) |
                                                (static_cast<unsigned char>(it[1]) << 8) | 
                                                (static_cast<unsigned char>(it[2]) << 16));
            if ((bom & 0xFFFFFF) == 0xBFBBEF)  
                count += 3;
        }
        return count;
    }

    static size_t codepoint_count(const char* it, 
                                  const char* end)
    {
        size_t count = 0;
        const char* p = it;
        while (p < end)
        {
            p += codepoint_length(p,end);
            ++count;
        }
        return count;
    }

    static uint32_t codepoint_at(const char* it, 
                                 const char* end,
                                 size_t index)
    {
        uint32_t cp = 0;
        const char* p = it;
        size_t count = 0;
        while (p < end && count <= index)
        {
            cp = char_sequence_to_codepoint(p,end,&p);
            ++count;
        }
        return cp;
    }

    static std::pair<const char*,size_t> char_sequence_at(const char* it, 
                                                          const char* end,
                                                          size_t index)
    {
        const char* p = it;
        size_t count = 0;
        
        while (p < end && count < index)
        {
            size_t length = codepoint_length(p,end);
            p += length;
            ++count;
        }
        size_t len = codepoint_length(p,end);
        return (count == index) ? std::make_pair(p,len) : std::make_pair(it,static_cast<size_t>(0));
    }

    static uint32_t char_sequence_to_codepoint(const char* it, 
                                               const char* end,
                                               const char** stop)
    {
        char c = *it;
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

    static size_t codepoint_length(const char* it, 
                                   const char* end)
    {
        size_t length = 0;
        char c = *it;
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

template <class CharT,size_t Size>
struct json_wchar_traits
{
};

template <>
struct json_wchar_traits<wchar_t,2> // assume utf16
{
    static size_t skip_bom(const wchar_t* it, size_t length)
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
            s.push_back(static_cast<wchar_t>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<wchar_t>((cp >> 10) + min_lead_surrogate - (0x10000 >> 10)));
            s.push_back(static_cast<wchar_t>((cp & 0x3ff) + min_trail_surrogate));
        }
    }

    static uint32_t char_sequence_to_codepoint(const wchar_t* it, const wchar_t* end, const wchar_t** stop)
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

    static size_t codepoint_length(const wchar_t* it, const wchar_t* end)
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

template <>
struct json_wchar_traits<wchar_t,4> // assume utf32
{
    static size_t skip_bom(const wchar_t* it, size_t length)
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
            s.push_back(static_cast<wchar_t>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<wchar_t>(cp));
        }
    }

    static uint32_t char_sequence_to_codepoint(const wchar_t* it, const wchar_t*, const wchar_t** stop)
    {
        uint32_t cp = static_cast<uint32_t>(*it);
        *stop = it + 1;
        return cp;
    }

    static size_t codepoint_length(const wchar_t* it, const wchar_t* end)
    {
        return (end > it) ? 1 : 0;
    }
};

template <>
struct json_text_traits<wchar_t> 
{
    static const std::pair<const wchar_t*,size_t>& null_literal() 
    {
        static const std::pair<const wchar_t*,size_t> value = {L"null",4};
        return value;
    }

    static const std::pair<const wchar_t*,size_t>& true_literal() 
    {
        static const std::pair<const wchar_t*,size_t> value = {L"true",4};
        return value;
    }

    static const std::pair<const wchar_t*,size_t>& false_literal() 
    {
        static const std::pair<const wchar_t*,size_t> value = {L"false",5};
        return value;
    }

    static size_t skip_bom(const wchar_t* it, size_t length)
    {
        return json_wchar_traits<wchar_t,sizeof(wchar_t)>::skip_bom(it, length);
    }

    static size_t codepoint_count(const wchar_t* it, 
                                  const wchar_t* end)
    {
        size_t count = 0;
        const wchar_t* p = it;
        while (p < end)
        {
            p += codepoint_length(p,end);
            ++count;
        }
        return count;
    }

    static std::pair<const wchar_t*,size_t> char_sequence_at(const wchar_t* it, 
                                                             const wchar_t* end,
                                                             size_t index)
    {
        const wchar_t* p = it;
        size_t count = 0;

        while (p < end && count < index)
        {
            size_t length = codepoint_length(p,end);
            p += length;
            ++count;
        }
        size_t len = codepoint_length(p,end);
        return (count == index) ? std::make_pair(p,len) : std::make_pair(it,static_cast<size_t>(0));
    }

    static uint32_t codepoint_at(const wchar_t* it, 
                                 const wchar_t* end,
                                 size_t index)
    {
        uint32_t cp = 0;
        const wchar_t* p = it;
        size_t count = 0;
        while (p < end && count <= index)
        {
            cp = char_sequence_to_codepoint(p,end,&p);
            ++count;
        }
        return cp;
    }

    static void append_codepoint_to_string(uint32_t cp, std::wstring& s)
    {
        json_wchar_traits<wchar_t,sizeof(wchar_t)>::append_codepoint_to_string(cp,s);
    }

    static uint32_t char_sequence_to_codepoint(const wchar_t* it, const wchar_t* end, const wchar_t** stop)
    {
        return json_wchar_traits<wchar_t,sizeof(wchar_t)>::char_sequence_to_codepoint(it, end, stop);
    }

    static size_t codepoint_length(const wchar_t* it, const wchar_t* end)
    {
        return json_wchar_traits<wchar_t, sizeof(wchar_t)>::codepoint_length(it, end);
    }
};

inline
bool is_control_character(uint32_t c)
{
    return c <= 0x1F || c == 0x7f;
}

inline
char to_hex_character(unsigned char c)
{
    JSONCONS_ASSERT(c <= 0xF);

    return (c < 10) ? ('0' + c) : ('A' - 10 + c);
}

inline
bool is_non_ascii_character(uint32_t c)
{
    return c >= 0x80;
}

template <class T>
struct type_wrapper
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

}
#endif
