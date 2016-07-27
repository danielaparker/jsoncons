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

template <class Json, class T>
class json_type_traits
{
public:
    static bool is(const Json&)
    {
        return false;
    }
};

template<class Json>
class json_type_traits<Json, typename Json::string_type>
{
public:
    typedef typename Json::string_type string_type;
    typedef typename string_type::allocator_type string_allocator;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static string_type as(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.as_string();
    }
    static string_type as(const Json& rhs, const string_allocator& allocator) JSONCONS_NOEXCEPT
    {
        return rhs.as_string(allocator);
    }
    static void assign(Json& lhs, const string_type& rhs)
    {
        lhs.assign_string(rhs);
    }
};

template<class Json>
class json_type_traits<Json, typename Json::any>
{
public:
    static bool is(const Json& lhs) JSONCONS_NOEXCEPT
    {
        return lhs.is_any();
    }
    static typename Json::any as(const Json& rhs)
    {
        return rhs.any_value();
    }
    static void assign(Json& lhs, typename Json::any rhs)
    {
        lhs.assign_any(rhs);
    }
};

template<class Json>
class json_type_traits<Json, typename type_wrapper<typename Json::char_type>::const_pointer_type>
{
public:
    typedef typename Json::char_type char_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const char_type* as(const Json& rhs)
    {
        return rhs.as_cstring();
    }
    static void assign(Json& lhs, const char_type *rhs)
    {
        size_t length = std::char_traits<char_type>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<class Json>
class json_type_traits<Json, typename type_wrapper<typename Json::char_type>::pointer_type>
{
public:
    typedef typename Json::char_type char_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static const char_type* as(const Json& rhs)
    {
        return rhs.as_cstring();
    }
    static void assign(Json& lhs, const char_type *rhs)
    {
        size_t length = std::char_traits<char_type>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<class Json>
class json_type_traits<Json, char>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static char as(const Json& rhs)
    {
        return static_cast<char>(rhs.as_integer());
    }
    static void assign(Json& lhs, char ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class Json>
class json_type_traits<Json, unsigned char>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static unsigned char as(const Json& rhs)
    {
        return static_cast<unsigned char>(rhs.as_uinteger());
    }
    static void assign(Json& lhs, unsigned char ch)
    {
        lhs.assign_uinteger(ch);
    }
};

template<class Json>
class json_type_traits<Json, signed char>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static signed char as(const Json& rhs)
    {
        return static_cast<signed char>(rhs.as_integer());
    }
    static void assign(Json& lhs, signed char ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class Json>
class json_type_traits<Json, wchar_t>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static wchar_t as(const Json& rhs)
    {
        return static_cast<wchar_t>(rhs.as_integer());
    }
    static void assign(Json& lhs, wchar_t ch)
    {
        lhs.assign_integer(ch);
    }
};

template<class Json>
class json_type_traits<Json, typename Json::object>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_object();
    }
    static typename Json::object as(Json rhs)
    {
        JSONCONS_ASSERT(rhs.is_object());
        return rhs.object_value();
    }
    static void assign(Json& lhs, typename Json::object rhs)
    {
        lhs.assign_object(rhs);
    }
};

template<class Json>
class json_type_traits<Json, typename Json::array>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_array();
    }
    static typename Json::array as(const Json& rhs)
    {
        JSONCONS_ASSERT(rhs.is_array());
        return rhs.array_value();
    }
    static void assign(Json& lhs, typename Json::array rhs)
    {
        lhs.assign_array(rhs);
    }
};

template<class Json>
class json_type_traits<Json, jsoncons::null_type>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_null();
    }
    static typename jsoncons::null_type as(const Json& rhs)
    {
        JSONCONS_ASSERT(rhs.is_null());
        return jsoncons::null_type();
    }
    static void assign(Json& lhs, null_type)
    {
        lhs.assign_null();
    }
};

template<class Json>
class json_type_traits<Json, bool>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const Json& rhs)
    {
        return rhs.as_bool();
    }
    static void assign(Json& lhs, bool rhs)
    {
        lhs.assign_bool(rhs);
    }
};

template<class Json>
class json_type_traits<Json, std::vector<bool>::reference>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_bool();
    }
    static bool as(const Json& rhs)
    {
        return rhs.as_bool();
    }
    static void assign(Json& lhs, std::vector<bool>::reference rhs)
    {
        lhs.assign_bool(rhs);
    }
};

template<class Json>
class json_type_traits<Json, short>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static short as(const Json& rhs)
    {
        return static_cast<short>(rhs.as_integer());
    }
    static void assign(Json& lhs, short rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class Json>
class json_type_traits<Json, unsigned short>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static unsigned short as(const Json& rhs)
    {
        return (unsigned short)rhs.as_uinteger();
    }
    static void assign(Json& lhs, unsigned short rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class Json>
class json_type_traits<Json, int>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static int as(const Json& rhs)
    {
        return static_cast<int>(rhs.as_integer());
    }
    static void assign(Json& lhs, int rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class Json>
class json_type_traits<Json, unsigned int>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static unsigned int as(const Json& rhs)
    {
        return static_cast<unsigned int>(rhs.as_uinteger());
    }
    static void assign(Json& lhs, unsigned int rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class Json>
class json_type_traits<Json, long>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static long as(const Json& rhs)
    {
        return static_cast<long>(rhs.as_integer());
    }
    static void assign(Json& lhs, long rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class Json>
class json_type_traits<Json, unsigned long>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static unsigned long as(const Json& rhs)
    {
        return static_cast<unsigned long>(rhs.as_uinteger());
    }
    static void assign(Json& lhs, unsigned long rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class Json>
class json_type_traits<Json, long long>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_integer();
    }
    static long long as(const Json& rhs)
    {
        return rhs.as_integer();
    }
    static void assign(Json& lhs, long long rhs)
    {
        lhs.assign_integer(rhs);
    }
};

template<class Json>
class json_type_traits<Json, unsigned long long>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_uinteger();
    }
    static unsigned long long as(const Json& rhs)
    {
        return rhs.as_uinteger();
    }
    static void assign(Json& lhs, unsigned long long rhs)
    {
        lhs.assign_uinteger(rhs);
    }
};

template<class Json>
class json_type_traits<Json, double>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }

    static double as(const Json& rhs)
    {
        return rhs.as_double();
    }
    static void assign(Json& lhs, double rhs)
    {
        lhs.assign_double(rhs);
    }
};

template<class Json>
class json_type_traits<Json, float>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_double();
    }
    static double as(const Json& rhs)
    {
        return static_cast<float>(rhs.as_double());
    }
    static void assign(Json& lhs, float rhs)
    {
        lhs.assign_double(static_cast<double>(rhs));
    }
};

template<class Json, typename T>
class json_type_traits<Json, std::vector<T>>
{
public:
    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
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
    static std::vector<T> as(const Json& rhs)
    {
        std::vector<T> v(rhs.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = rhs[i].template as<T>();
        }
        return v;
    }
    static void assign(Json& lhs, const std::vector<T>& rhs)
    {
        lhs = Json(rhs.begin(), rhs.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
