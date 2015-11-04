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
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_string();
    }
    std::basic_string<Char> as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_string();
    }
    void assign(basic_json<Char, Alloc>& self, const std::basic_string<Char>& val)
    {
        self.assign_string(val);
    }
};

/*
template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, basic_json<Char, Alloc>>
{
public:
    bool is(const basic_json<Char, Alloc>&) const
    {
        return true;
    }
    basic_json<Char, Alloc> as(const basic_json<Char, Alloc>& val) const
    {
        return val;
    }
    void assign(basic_json<Char, Alloc>& self, basic_json<Char, Alloc> val)
    {
        val.swap(self);
    }
};*/

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::any>
{
public:
    bool is(const basic_json<Char, Alloc>& self) const
    {
        return self.is_any();
    }
    typename basic_json<Char, Alloc>::any as(const basic_json<Char, Alloc>& val) const
    {
        return val.any_value();
    }
    void assign(basic_json<Char, Alloc>& self, typename basic_json<Char, Alloc>::any val)
    {
        self.assign_any(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, const Char *>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_string();
    }
    const Char* as(const basic_json<Char, Alloc>& val) const
    {
        return val.value.as_c_str();
    }
    void assign(basic_json<Char, Alloc>& self, const Char *val)
    {
        self.assign_string(std::basic_string<Char>(val));
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::object>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_object();
    }
    typename basic_json<Char, Alloc>::object as(const basic_json<Char, Alloc>& val) const
    {
        return typename basic_json<Char, Alloc>::object(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, typename basic_json<Char, Alloc>::array>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_array();
    }
    typename basic_json<Char, Alloc>::array as(const basic_json<Char, Alloc>& val) const
    {
        return typename basic_json<Char, Alloc>::array(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, jsoncons::null_type>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_null();
    }
    typename jsoncons::null_type as(const basic_json<Char, Alloc>& val) const
    {
        JSONCONS_ASSERT(val.is_null());
        return jsoncons::null_type();
    }
    void assign(basic_json<Char, Alloc>& self, null_type)
    {
        self.assign_null();
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, bool>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_bool();
    }
    bool as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_bool();
    }
    void assign(basic_json<Char, Alloc>& self, bool val)
    {
        self.assign_bool(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, short>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    short as(const basic_json<Char, Alloc>& val) const
    {
        return (short)val.as_int();
    }
    void assign(basic_json<Char, Alloc>& self, short val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned short>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    unsigned short as(const basic_json<Char, Alloc>& val) const
    {
        return (unsigned short)val.as_uint();
    }
    void assign(basic_json<Char, Alloc>& self, unsigned short val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, int>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    int as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_int();
    }
    void assign(basic_json<Char, Alloc>& self, int val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned int>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    unsigned int as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_uint();
    }
    void assign(basic_json<Char, Alloc>& self, unsigned int val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    long as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_long();
    }
    void assign(basic_json<Char, Alloc>& self, long val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    unsigned long as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_ulong();
    }
    void assign(basic_json<Char, Alloc>& self, unsigned long val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, long long>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    long long as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_longlong();
    }
    void assign(basic_json<Char, Alloc>& self, long long val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, unsigned long long>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    unsigned long long as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_ulonglong();
    }
    void assign(basic_json<Char, Alloc>& self, unsigned long long val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, Char>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_string() && val.as_string().size() == 1;
    }
    Char as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_char();
    }
    void assign(basic_json<Char, Alloc>& self, Char ch)
    {
        std::basic_string<Char> s;
        s.push_back(ch);
        self.assign_string(s);
    }
};

template<typename Char, typename Alloc>
class json_type_traits<Char, Alloc, double>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
    {
        return val.is_double();
    }

    double as(const basic_json<Char, Alloc>& val) const
    {
        return val.as_double();
    }
    void assign(basic_json<Char, Alloc>& self, double val)
    {
        self.assign_double(val);
    }
};

template<typename Char, typename Alloc, typename T>
class json_type_traits<Char, Alloc, std::vector<T>>
{
public:
    bool is(const basic_json<Char, Alloc>& val) const
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
    std::vector<T> as(const basic_json<Char, Alloc>& val) const
    {
        std::vector<T> v(val.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = val[i].template as<T>();
        }
        return v;
    }
    void assign(basic_json<Char, Alloc>& self, const std::vector<T>& val)
    {
        self = basic_json<Char, Alloc>(val.begin(), val.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
