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
class json_type_traits<Char, Alloc, std::basic_string<Char>>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_string();
    }
    static std::basic_string<Char> as(const basic_json<Char, Alloc>& val)
    {
        return val.as_string();
    }
    static void assign(basic_json<Char, Alloc>& lhs, const std::basic_string<Char>& val)
    {
        lhs.assign_string(val);
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
    static typename basic_json<Char, Alloc>::any as(const basic_json<Char, Alloc>& val)
    {
        return val.any_value();
    }
    static void assign(basic_json<Char, Alloc>& lhs, typename basic_json<Char, Alloc>::any val)
    {
        lhs.assign_any(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, const Char *>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_string();
    }
    static const Char* as(const basic_json<Char, Alloc>& val)
    {
        return val.value.as_c_str();
    }
    static void assign(basic_json<Char, Alloc>& lhs, const Char *val)
    {
        lhs.assign_cstring(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, Char>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_string() && val.as_string().size() == 1;
    }
    static Char as(const basic_json<Char, Alloc>& val)
    {
        return val.as_char();
    }
    static void assign(basic_json<Char, Alloc>& lhs, Char ch)
    {
        std::basic_string<Char> s;
        s.push_back(ch);
        lhs.assign_string(s);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::object>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_object();
    }
    static typename basic_json<Char, Alloc>::object as(const basic_json<Char, Alloc>& val)
    {
        return typename basic_json<Char, Alloc>::object(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::array>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_array();
    }
    static typename basic_json<Char, Alloc>::array as(const basic_json<Char, Alloc>& val)
    {
        return typename basic_json<Char, Alloc>::array(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, jsoncons::null_type>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_null();
    }
    static typename jsoncons::null_type as(const basic_json<Char, Alloc>& val)
    {
        JSONCONS_ASSERT(val.is_null());
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
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_bool();
    }
    static bool as(const basic_json<Char, Alloc>& val)
    {
        return val.as_bool();
    }
    static void assign(basic_json<Char, Alloc>& lhs, bool val)
    {
        lhs.assign_bool(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, short>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static short as(const basic_json<Char, Alloc>& val)
    {
        return (short)val.as_int();
    }
    static void assign(basic_json<Char, Alloc>& lhs, short val)
    {
        lhs.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned short>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && static_cast<unsigned long long>(val.as_longlong()) <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned short as(const basic_json<Char, Alloc>& val)
    {
        return (unsigned short)val.as_uint();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned short val)
    {
        lhs.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, int>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static int as(const basic_json<Char, Alloc>& val)
    {
        return val.as_int();
    }
    static void assign(basic_json<Char, Alloc>& lhs, int val)
    {
        lhs.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned int>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && static_cast<unsigned long long>(val.as_longlong()) <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned int as(const basic_json<Char, Alloc>& val)
    {
        return val.as_uint();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned int val)
    {
        lhs.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long as(const basic_json<Char, Alloc>& val)
    {
        return val.as_long();
    }
    static void assign(basic_json<Char, Alloc>& lhs, long val)
    {
        lhs.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && static_cast<unsigned long long>(val.as_longlong()) <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static unsigned long as(const basic_json<Char, Alloc>& val)
    {
        return val.as_ulong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned long val)
    {
        lhs.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long long>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return true;
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= static_cast<unsigned long long>(std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP());
        }
        else
        {
            return false;
        }
    }
    static long long as(const basic_json<Char, Alloc>& val)
    {
        return val.as_longlong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, long long val)
    {
        lhs.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long long>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && static_cast<unsigned long long>(val.as_longlong()) <= std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    static unsigned long long as(const basic_json<Char, Alloc>& val)
    {
        return val.as_ulonglong();
    }
    static void assign(basic_json<Char, Alloc>& lhs, unsigned long long val)
    {
        lhs.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, double>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        return val.is_double();
    }

    static double as(const basic_json<Char, Alloc>& val)
    {
        return val.as_double();
    }
    static void assign(basic_json<Char, Alloc>& lhs, double val)
    {
        lhs.assign_double(val);
    }
};

template<typename Char, typename Alloc, typename T>
class json_type_traits<Char, Alloc, std::vector<T>>
{
public:
    static bool is(const basic_json<Char, Alloc>& val)
    {
        bool result = val.is_array();
        for (size_t i = 0; result && i < val.size(); ++i)
        {
            if (!val[i].template is<T>())
            {
                result = false;
            }
        }
        return result;
    }
    static std::vector<T> as(const basic_json<Char, Alloc>& val)
    {
        std::vector<T> v(val.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = val[i].template as<T>();
        }
        return v;
    }
    static void assign(basic_json<Char, Alloc>& lhs, const std::vector<T>& val)
    {
        lhs = basic_json<Char, Alloc>(val.begin(), val.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
