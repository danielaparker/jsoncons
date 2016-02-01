// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <fstream>
#include <limits>
#include "jsoncons/json1.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif


namespace jsoncons {

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, std::basic_string<CharT>>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static std::basic_string<CharT> as(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.as_string();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, const std::basic_string<CharT>& rhs)
    {
        lhs.assign_string(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, typename basic_json<CharT, Alloc>::any>
{
public:
    static bool is(const basic_json<CharT, Alloc>& lhs) JSONCONS_NOEXCEPT
    {
        return lhs.is_any();
    }
    static typename basic_json<CharT, Alloc>::any as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.any_value();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, typename basic_json<CharT, Alloc>::any rhs)
    {
        lhs.assign_any(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, typename type_wrapper<CharT>::const_pointer_type>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const CharT* as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_cstring();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, const CharT *rhs)
    {
        size_t length = std::char_traits<CharT>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, char>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<char>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static char as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<char>(rhs.as_integer());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, char ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, unsigned char>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<unsigned long long>(rhs.as_integer()) <= std::numeric_limits<unsigned char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= std::numeric_limits<unsigned char>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned char as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<unsigned char>(rhs.as_uinteger());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, unsigned char ch)
    {
        lhs.assign_ulonglong(ch);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, signed char>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<char>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static signed char as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<signed char>(rhs.as_integer());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, signed char ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, wchar_t>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<wchar_t>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<wchar_t>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<wchar_t>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static wchar_t as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<wchar_t>(rhs.as_integer());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, wchar_t ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, typename basic_json<CharT, Alloc>::object>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_object();
    }
    static typename basic_json<CharT, Alloc>::object as(basic_json<CharT, Alloc> rhs)
    {
		JSONCONS_ASSERT(rhs.is_object());
		return rhs.object_value();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, typename basic_json<CharT, Alloc>::object rhs)
    {
        lhs.assign_object(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, typename basic_json<CharT, Alloc>::array>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_array();
    }
    static typename basic_json<CharT, Alloc>::array as(const basic_json<CharT, Alloc>& rhs)
    {
		JSONCONS_ASSERT(rhs.is_array());
        return rhs.array_value();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, typename basic_json<CharT, Alloc>::array rhs)
    {
        lhs.assign_array(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, jsoncons::null_type>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_null();
    }
    static typename jsoncons::null_type as(const basic_json<CharT, Alloc>& rhs)
    {
        JSONCONS_ASSERT(rhs.is_null());
        return jsoncons::null_type();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, null_type)
    {
        lhs.assign_null();
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, bool>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_bool();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, bool rhs)
    {
        lhs.assign_bool(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, short>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static short as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<short>(rhs.as_int());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, short rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, unsigned short>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<unsigned long long>(rhs.as_integer()) <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned short as(const basic_json<CharT, Alloc>& rhs)
    {
        return (unsigned short)rhs.as_uint();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, unsigned short rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, int>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static int as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_int();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, int rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, unsigned int>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<unsigned long long>(rhs.as_integer()) <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned int as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_uint();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, unsigned int rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, long>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_long();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, long rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, unsigned long>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<unsigned long long>(rhs.as_integer()) <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned long as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_ulong();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, unsigned long rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, long long>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return true;
        }
        else if (rhs.is_uinteger())
        {
            return rhs.as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long long as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_integer();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, long long rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, unsigned long long>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= 0 && static_cast<unsigned long long>(rhs.as_integer()) <= std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_uinteger())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    static unsigned long long as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_uinteger();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, unsigned long long rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, double>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }

    static double as(const basic_json<CharT, Alloc>& rhs)
    {
        return rhs.as_double();
    }
    static void assign(basic_json<CharT, Alloc>& lhs, double rhs)
    {
        lhs.assign_double(rhs);
    }
};

template<typename CharT, typename Alloc>
class json_type_traits<CharT, Alloc, float>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }
    static double as(const basic_json<CharT, Alloc>& rhs)
    {
        return static_cast<float>(rhs.as_double());
    }
    static void assign(basic_json<CharT, Alloc>& lhs, float rhs)
    {
        lhs.assign_double(static_cast<double>(rhs));
    }
};

template<typename CharT, typename Alloc, typename T>
class json_type_traits<CharT, Alloc, std::vector<T>>
{
public:
    static bool is(const basic_json<CharT, Alloc>& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_array();
        for (size_t i = 0; result && i < rhs.size(); ++i)
        {
            if (!rhs[i].template is<T>())
            {
                result = false;
            }
        }
        return result;
    }
    static std::vector<T> as(const basic_json<CharT, Alloc>& rhs)
    {
        std::vector<T> v(rhs.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = rhs[i].template as<T>();
        }
        return v;
    }
    static void assign(basic_json<CharT, Alloc>& lhs, const std::vector<T>& rhs)
    {
        lhs = basic_json<CharT, Alloc>(rhs.begin(), rhs.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
