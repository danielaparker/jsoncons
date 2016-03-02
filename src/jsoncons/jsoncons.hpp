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

template <typename Base>
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

template <typename Base>
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

// json_char_traits

const uint16_t min_lead_surrogate = 0xD800;
const uint16_t max_lead_surrogate = 0xDBFF;
const uint16_t min_trail_surrogate = 0xDC00;
const uint16_t max_trail_surrogate = 0xDFFF;

template <typename CharT>
struct json_literals
{
};

template <>
struct json_literals<char>
{
    static std::pair<const char*,size_t> null_literal() 
    {
        static const char* value = "null";
        return std::pair<const char*,size_t>(value,4);
    }

    static std::pair<const char*,size_t> true_literal() 
    {
        static const char* value = "true";
        return std::pair<const char*,size_t>(value,4);
    }

    static std::pair<const char*,size_t> false_literal() 
    {
        static const char* value = "false";
        return std::pair<const char*,size_t>(value,5);
    }
};

template <>
struct json_literals<wchar_t> 
{
    static std::pair<const wchar_t*,size_t> null_literal() 
    {
        static const wchar_t* value = L"null";
        return std::pair<const wchar_t*,size_t>(value,4);
    }

    static std::pair<const wchar_t*,size_t> true_literal() 
    {
        static const wchar_t* value = L"true";
        return std::pair<const wchar_t*,size_t>(value,4);
    }

    static std::pair<const wchar_t*,size_t> false_literal() 
    {
        static const wchar_t* value = L"false";
        return std::pair<const wchar_t*,size_t>(value,5);
    }
};

template <typename CharT,size_t Size>
struct json_char_traits
{
};

template <>
struct json_char_traits<char,1>
{
    static uint32_t convert_char_to_codepoint(const char*& it, 
                                              const char*)
    {
        char c = *it;
        uint32_t u(c >= 0 ? c : 256 + c );
        uint32_t cp = u;
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
            cp = ((cp << 12) & 0xffff) + ((static_cast<uint32_t>(0xff & u) << 6) & 0xfff);
            c = *(++it);
            u = (c >= 0 ? c : 256 + c );
            cp += (u) & 0x3f;
        }
        else if ((u >> 3) == 0x1e)
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
        }
        else
        {
        }
        return cp;
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

template <>
struct json_char_traits<wchar_t,2> // assume utf16
{
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

    static uint32_t convert_char_to_codepoint(const wchar_t*& it, const wchar_t*)
    {
        uint32_t cp = (0xffff & *it);
        if ((cp >= min_lead_surrogate && cp <= max_lead_surrogate)) // surrogate pair
        {
            uint32_t trail_surrogate = 0xffff & *(++it);
            cp = (cp << 10) + trail_surrogate + 0x10000u - (min_lead_surrogate << 10) - min_trail_surrogate;
        }
        return cp;
    }
};

template <>
struct json_char_traits<wchar_t,4> // assume utf32
{
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

    static uint32_t convert_char_to_codepoint(const wchar_t*& it, const wchar_t*)
    {
        uint32_t cp = static_cast<uint32_t>(*it);
        return cp;
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

template <typename T>
struct type_wrapper
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <typename T>
struct type_wrapper<const T>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <typename T>
struct type_wrapper<T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <typename T>
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
