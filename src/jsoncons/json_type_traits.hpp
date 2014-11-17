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

template<typename Char, typename Storage>
class value_adapter<Char, Storage, std::basic_string<Char>>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_string();
    }
    std::basic_string<Char> as(const basic_json<Char, Storage>& val) const
    {
        return val.as_string();
    }
    void assign(basic_json<Char, Storage>& self, const std::basic_string<Char>& val)
    {
        self.assign_string(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, basic_json<Char, Storage>>
{
public:
    bool is(const basic_json<Char, Storage>&) const
    {
        return true;
    }
    basic_json<Char, Storage> as(const basic_json<Char, Storage>& val) const
    {
        return val;
    }
    void assign(basic_json<Char, Storage>& self, basic_json<Char, Storage> val)
    {
        val.swap(self);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, typename basic_json<Char, Storage>::any>
{
public:
    bool is(const basic_json<Char, Storage>&) const
    {
        return true;
    }
    typename basic_json<Char, Storage>::any as(const basic_json<Char, Storage>& val) const
    {
        return val;
    }
    void assign(basic_json<Char, Storage>& self, typename basic_json<Char, Storage>::any val)
    {
        self.assign_any(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, const Char *>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_string();
    }
    const Char* as(const basic_json<Char, Storage>& val) const
    {
        JSONCONS_ASSERT(val.is_string());
        return val.value_.value_string_.c_str();
    }
    void assign(basic_json<Char, Storage>& self, const Char *val)
    {
        self.assign_string(std::basic_string<Char>(val));
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, typename basic_json<Char, Storage>::object>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_object();
    }
    typename basic_json<Char, Storage>::object as(const basic_json<Char, Storage>& val) const
    {
        return val;
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, typename basic_json<Char, Storage>::array>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_array();
    }
    typename basic_json<Char, Storage>::array as(const basic_json<Char, Storage>& val) const
    {
        return val;
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, jsoncons::null_type>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_null();
    }
    typename jsoncons::null_type as(const basic_json<Char, Storage>& val) const
    {
        JSONCONS_ASSERT(val.is_null());
        return jsoncons::null_type();
    }
    void assign(basic_json<Char, Storage>& self, null_type)
    {
        self.assign_null();
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, bool>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_bool();
    }
    bool as(const basic_json<Char, Storage>& val) const
    {
        return val.as_bool();
    }
    void assign(basic_json<Char, Storage>& self, bool val)
    {
        self.assign_bool(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, int>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    int as(const basic_json<Char, Storage>& val) const
    {
        return val.as_int();
    }
    void assign(basic_json<Char, Storage>& self, int val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, unsigned int>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
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
    unsigned int as(const basic_json<Char, Storage>& val) const
    {
        return val.as_uint();
    }
    void assign(basic_json<Char, Storage>& self, unsigned int val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, short>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    short as(const basic_json<Char, Storage>& val) const
    {
        return (short)val.as_int();
    }
    void assign(basic_json<Char, Storage>& self, short val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, unsigned short>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
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
    unsigned short as(const basic_json<Char, Storage>& val) const
    {
        return (unsigned short)val.as_uint();
    }
    void assign(basic_json<Char, Storage>& self, unsigned short val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, long>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    long as(const basic_json<Char, Storage>& val) const
    {
        return val.as_long();
    }
    void assign(basic_json<Char, Storage>& self, long val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, unsigned long>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
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
    unsigned long as(const basic_json<Char, Storage>& val) const
    {
        return val.as_ulong();
    }
    void assign(basic_json<Char, Storage>& self, unsigned long val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, Char>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_string() && val.as_string().size() == 1;
    }
    char as(const basic_json<Char, Storage>& val) const
    {
        return val.as_char();
    }
    void assign(basic_json<Char, Storage>& self, Char ch)
    {
        std::basic_string<Char> s;
        s.push_back(ch);
        self.assign_string(s);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, double>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        return val.is_double();
    }

    double as(const basic_json<Char, Storage>& val) const
    {
        return val.as_double();
    }
    void assign(basic_json<Char, Storage>& self, double val)
    {
        self.assign_double(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, long long>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return true;
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    long long as(const basic_json<Char, Storage>& val) const
    {
        return val.as_longlong();
    }
    void assign(basic_json<Char, Storage>& self, long long val)
    {
        self.assign_longlong(val);
    }
};

template<typename Char, typename Storage>
class value_adapter<Char, Storage, unsigned long long>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0;
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
    unsigned long long as(const basic_json<Char, Storage>& val) const
    {
        return val.as_ulonglong();
    }
    void assign(basic_json<Char, Storage>& self, unsigned long long val)
    {
        self.assign_ulonglong(val);
    }
};

template<typename Char, typename Storage, typename T>
class value_adapter<Char, Storage, std::vector<T>>
{
public:
    bool is(const basic_json<Char, Storage>& val) const
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
    std::vector<T> as(const basic_json<Char, Storage>& val) const
    {
        std::vector<T> v(val.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = val[i].template as<T>();
        }
        return v;
    }
    void assign(basic_json<Char, Storage>& self, const std::vector<T>& val)
    {
        self = basic_json<Char, Storage>(val.begin(), val.end());
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
