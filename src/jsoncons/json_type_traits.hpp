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
#include "jsoncons/jsoncons.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

template <class JsonT, typename T>
class json_type_traits
{
public:
    static bool is(const JsonT&)
    {
        return false;
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename JsonT::string_type>
{
public:
    typedef typename JsonT::string_type string_type;
    typedef typename string_type::allocator_type string_allocator;

    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static string_type as(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.as_string();
    }
    static string_type as(const JsonT& rhs, const string_allocator& allocator) JSONCONS_NOEXCEPT
    {
        return rhs.as_string(allocator);
    }
    static void assign(JsonT& lhs, const string_type& rhs)
    {
        lhs.assign_string(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename JsonT::any>
{
public:
    static bool is(const JsonT& lhs) JSONCONS_NOEXCEPT
    {
        return lhs.is_any();
    }
    static typename JsonT::any as(const JsonT& rhs)
    {
        return rhs.any_value();
    }
    static void assign(JsonT& lhs, typename JsonT::any rhs)
    {
        lhs.assign_any(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename type_wrapper<typename JsonT::char_type>::const_pointer_type>
{
public:
    typedef typename JsonT::char_type char_type;

    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const char_type* as(const JsonT& rhs)
    {
        return rhs.as_cstring();
    }
    static void assign(JsonT& lhs, const char_type *rhs)
    {
        size_t length = std::char_traits<char_type>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename type_wrapper<typename JsonT::char_type>::pointer_type>
{
public:
    typedef typename JsonT::char_type char_type;

    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const char_type* as(const JsonT& rhs)
    {
        return rhs.as_cstring();
    }
    static void assign(JsonT& lhs, const char_type *rhs)
    {
        size_t length = std::char_traits<char_type>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<class JsonT>
class json_type_traits<JsonT, char>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        if (rhs.is_integer())
        {
            return rhs.as_integer() >= std::numeric_limits<char>::min JSONCONS_NO_MACRO_EXP() && rhs.as_integer() <= std::numeric_limits<char>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    static char as(const JsonT& rhs)
    {
        return static_cast<char>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, char ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class JsonT>
class json_type_traits<JsonT, unsigned char>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static unsigned char as(const JsonT& rhs)
    {
        return static_cast<unsigned char>(rhs.as_uinteger());
    }
    static void assign(JsonT& lhs, unsigned char ch)
    {
        lhs.assign_uinteger(ch);
    }
};

template<class JsonT>
class json_type_traits<JsonT, signed char>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static signed char as(const JsonT& rhs)
    {
        return static_cast<signed char>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, signed char ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class JsonT>
class json_type_traits<JsonT, wchar_t>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static wchar_t as(const JsonT& rhs)
    {
        return static_cast<wchar_t>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, wchar_t ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename JsonT::object>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_object();
    }
    static typename JsonT::object as(JsonT rhs)
    {
        JSONCONS_ASSERT(rhs.is_object());
        return rhs.object_value();
    }
    static void assign(JsonT& lhs, typename JsonT::object rhs)
    {
        lhs.assign_object(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, typename JsonT::array>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_array();
    }
    static typename JsonT::array as(const JsonT& rhs)
    {
        JSONCONS_ASSERT(rhs.is_array());
        return rhs.array_value();
    }
    static void assign(JsonT& lhs, typename JsonT::array rhs)
    {
        lhs.assign_array(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, jsoncons::null_type>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_null();
    }
    static typename jsoncons::null_type as(const JsonT& rhs)
    {
        JSONCONS_ASSERT(rhs.is_null());
        return jsoncons::null_type();
    }
    static void assign(JsonT& lhs, null_type)
    {
        lhs.assign_null();
    }
};

template<class JsonT>
class json_type_traits<JsonT, bool>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const JsonT& rhs)
    {
        return rhs.as_bool();
    }
    static void assign(JsonT& lhs, bool rhs)
    {
        lhs.assign_bool(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, short>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static short as(const JsonT& rhs)
    {
        return static_cast<short>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, short rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, unsigned short>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static unsigned short as(const JsonT& rhs)
    {
        return (unsigned short)rhs.as_uinteger();
    }
    static void assign(JsonT& lhs, unsigned short rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, int>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static int as(const JsonT& rhs)
    {
        return static_cast<int>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, int rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, unsigned int>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static unsigned int as(const JsonT& rhs)
    {
        return static_cast<unsigned int>(rhs.as_uinteger());
    }
    static void assign(JsonT& lhs, unsigned int rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, long>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static long as(const JsonT& rhs)
    {
        return static_cast<long>(rhs.as_integer());
    }
    static void assign(JsonT& lhs, long rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, unsigned long>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static unsigned long as(const JsonT& rhs)
    {
        return static_cast<unsigned long>(rhs.as_uinteger());
    }
    static void assign(JsonT& lhs, unsigned long rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, long long>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_integer();
    }
    static long long as(const JsonT& rhs)
    {
        return rhs.as_integer();
    }
    static void assign(JsonT& lhs, long long rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, unsigned long long>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_uinteger();
    }
    static unsigned long long as(const JsonT& rhs)
    {
        return rhs.as_uinteger();
    }
    static void assign(JsonT& lhs, unsigned long long rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, double>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }

    static double as(const JsonT& rhs)
    {
        return rhs.as_double();
    }
    static void assign(JsonT& lhs, double rhs)
    {
        lhs.assign_double(rhs);
    }
};

template<class JsonT>
class json_type_traits<JsonT, float>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }
    static double as(const JsonT& rhs)
    {
        return static_cast<float>(rhs.as_double());
    }
    static void assign(JsonT& lhs, float rhs)
    {
        lhs.assign_double(static_cast<double>(rhs));
    }
};

template<class JsonT, typename T>
class json_type_traits<JsonT, std::vector<T>>
{
public:
    static bool is(const JsonT& rhs) JSONCONS_NOEXCEPT
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
    static std::vector<T> as(const JsonT& rhs)
    {
        std::vector<T> v(rhs.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = rhs[i].template as<T>();
        }
        return v;
    }
    static void assign(JsonT& lhs, const std::vector<T>& rhs)
    {
        lhs = JsonT(rhs.begin(), rhs.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
