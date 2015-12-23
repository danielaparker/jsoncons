// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::object_key_proxy>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return true;
    }
    static basic_json<Char, Alloc> as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs;
    }
    static void assign(basic_json<Char, Alloc>& lhs, basic_json<Char, Alloc> rhs)
    {
        lhs.swap(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, std::basic_string<Char>>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_string();
    }
    static std::basic_string<Char> as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_string();
    }
    static void assign(basic_json<Char, Alloc>& lhs, const std::basic_string<Char>& rhs)
    {
        lhs.assign_string(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::any>
{
public:
    static bool is(const basic_json<Char, Alloc>& lhs)
    {
        return lhs.is_any();
    }
    static typename basic_json<Char, Alloc>::any as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.any_value();
    }
    static void assign(basic_json<Char, Alloc>& lhs, typename basic_json<Char, Alloc>::any rhs)
    {
        lhs.assign_any(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, const Char *>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_string();
    }
    static const Char* as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.value.as_cstring();
    }
    static void assign(basic_json<Char, Alloc>& lhs, const Char *rhs)
    {
        size_t length = std::char_traits<Char>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, char>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<char>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static char as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<char>(rhs.as_longlong());
    }
    static void assign(basic_json<Char, Alloc>& lhs, char ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned char>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= 0 && static_cast<unsigned long long>(rhs.as_longlong()) <= std::numeric_limits<unsigned char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= std::numeric_limits<unsigned char>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned char as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<unsigned char>(rhs.as_ulonglong());
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned char ch)
    {
        lhs.assign_ulonglong(ch);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, signed char>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<char>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static signed char as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<signed char>(rhs.as_longlong());
    }
    static void assign(basic_json<Char, Alloc>& lhs, signed char ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, wchar_t>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<wchar_t>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<wchar_t>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<wchar_t>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static wchar_t as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<wchar_t>(rhs.as_longlong());
    }
    static void assign(basic_json<Char, Alloc>& lhs, wchar_t ch)
    {
        lhs.assign_longlong(ch);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::object>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_object();
    }
    static typename basic_json<Char, Alloc>::object as(basic_json<Char, Alloc> rhs)
    {
		JSONCONS_ASSERT(rhs.is_object());
		return rhs.members();
    }
    static void assign(basic_json<Char, Alloc>& lhs, typename basic_json<Char, Alloc>::object rhs)
    {
        lhs.assign_object(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::array>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_array();
    }
    static typename basic_json<Char, Alloc>::array as(const basic_json<Char, Alloc>& rhs)
    {
		JSONCONS_ASSERT(rhs.is_array());
        return rhs.elements();
    }
    static void assign(basic_json<Char, Alloc>& lhs, typename basic_json<Char, Alloc>::array rhs)
    {
        lhs.assign_array(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, jsoncons::null_type>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_null();
    }
    static typename jsoncons::null_type as(const basic_json<Char, Alloc>& rhs)
    {
        JSONCONS_ASSERT(rhs.is_null());
        return jsoncons::null_type();
    }
    static void assign(basic_json<Char, Alloc>& lhs, null_type)
    {
        lhs.assign_null();
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, bool>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_bool();
    }
    static bool as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_bool();
    }
    static void assign(basic_json<Char, Alloc>& lhs, bool rhs)
    {
        lhs.assign_bool(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, short>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static short as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<short>(rhs.as_int());
    }
    static void assign(basic_json<Char, Alloc>& lhs, short rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned short>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= 0 && static_cast<unsigned long long>(rhs.as_longlong()) <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned short as(const basic_json<Char, Alloc>& rhs)
    {
        return (unsigned short)rhs.as_uint();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned short rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, int>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static int as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_int();
    }
    static void assign(basic_json<Char, Alloc>& lhs, int rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned int>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= 0 && static_cast<unsigned long long>(rhs.as_longlong()) <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned int as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_uint();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned int rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXP() && rhs.as_longlong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_long();
    }
    static void assign(basic_json<Char, Alloc>& lhs, long rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= 0 && static_cast<unsigned long long>(rhs.as_longlong()) <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned long as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_ulong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned long rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long long>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return true;
        }
        else if (rhs.is_ulonglong())
        {
            return rhs.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long long as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_longlong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, long long rhs)
    {
        lhs.assign_longlong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long long>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        if (rhs.is_longlong())
        {
            return rhs.as_longlong() >= 0 && static_cast<unsigned long long>(rhs.as_longlong()) <= std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (rhs.is_ulonglong())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    static unsigned long long as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_ulonglong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned long long rhs)
    {
        lhs.assign_ulonglong(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, double>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_double();
    }

    static double as(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.as_double();
    }
    static void assign(basic_json<Char, Alloc>& lhs, double rhs)
    {
        lhs.assign_double(rhs);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, float>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
    {
        return rhs.is_double();
    }
    static double as(const basic_json<Char, Alloc>& rhs)
    {
        return static_cast<float>(rhs.as_double());
    }
    static void assign(basic_json<Char, Alloc>& lhs, float rhs)
    {
        lhs.assign_double(static_cast<double>(rhs));
    }
};

template<typename Char, typename Alloc, typename T>
class json_type_traits<Char, Alloc, std::vector<T>>
{
public:
    static bool is(const basic_json<Char, Alloc>& rhs)
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
    static std::vector<T> as(const basic_json<Char, Alloc>& rhs)
    {
        std::vector<T> v(rhs.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = rhs[i].template as<T>();
        }
        return v;
    }
    static void assign(basic_json<Char, Alloc>& lhs, const std::vector<T>& rhs)
    {
        lhs = basic_json<Char, Alloc>(rhs.begin(), rhs.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
