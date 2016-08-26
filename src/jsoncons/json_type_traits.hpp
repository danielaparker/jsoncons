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
#include <type_traits>
#include "jsoncons/jsoncons.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

template <class Json, class T, class Enable=void>
struct json_string_type_traits
{
    typedef void char_traits_type;
};

template <class Json, class T>
struct json_string_type_traits<Json, T, typename std::enable_if<std::is_same<typename T::traits_type,typename Json::char_traits_type>::value>::type>
{
    typedef typename Json::char_traits_type char_traits_type;
};

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    static const bool is_assignable = false;

    static bool is(const Json&)
    {
        return false;
    }

    static T as(const Json& rhs);

    static void assign(Json& lhs, T rhs);
};

template <class Json, class T>
class json_array_input_iterator
{
public:
    typedef typename Json::const_array_iterator base_iterator;
    typedef typename std::iterator_traits<base_iterator>::value_type value_type;
    typedef typename std::iterator_traits<base_iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<base_iterator>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;

    json_array_input_iterator()
    {
    }

    json_array_input_iterator(base_iterator it)
        : it_(it)
    {
    }

    json_array_input_iterator& operator=(json_array_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_array_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_array_input_iterator operator++(int) // postfix increment
    {
        json_array_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_array_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_array_input_iterator operator--(int)
    {
        json_array_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return json_type_traits<Json,T>::as(*it_);
    }

    friend bool operator==(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_array_input_iterator& lhs, json_array_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    base_iterator it_;
};

template <class Json, class T>
class json_object_input_iterator
{
public:
    typedef typename Json::const_object_iterator base_iterator;
    typedef typename std::iterator_traits<base_iterator>::value_type value_type;
    typedef typename std::iterator_traits<base_iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<base_iterator>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;
    typedef typename T::second_type mapped_type;

    json_object_input_iterator()
    {
    }

    json_object_input_iterator(base_iterator it)
        : it_(it)
    {
    }

    json_object_input_iterator& operator=(json_object_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_object_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_object_input_iterator operator++(int) // postfix increment
    {
        json_object_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_object_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_object_input_iterator operator--(int)
    {
        json_object_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return T(it_->name(),json_type_traits<Json,mapped_type>::as(it_->value()));
    }

    friend bool operator==(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_object_input_iterator& lhs, json_object_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    base_iterator it_;
};

template<class Json>
struct json_type_traits<Json, typename Json::any>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::const_pointer_type>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::pointer_type>
{
    static const bool is_assignable = true;

    typedef typename Json::char_type char_type;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }
    static void assign(Json& lhs, const char_type *rhs)
    {
        size_t length = std::char_traits<char_type>::length(rhs);
        lhs.assign_string(rhs,length);
    }
};

template<class Json>
struct json_type_traits<Json, char>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, unsigned char>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, signed char>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, wchar_t>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, typename Json::object>
{
    static const bool is_assignable = true;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_object();
    }
    static void assign(Json& lhs, typename Json::object rhs)
    {
        lhs.assign_object(rhs);
    }
};

template<class Json>
struct json_type_traits<Json, Json>
{
    static const bool is_assignable = true;

    static bool is(const Json&) JSONCONS_NOEXCEPT
    {
        return true;
    }
    static Json as(Json rhs)
    {
        return rhs;
    }
    static void assign(Json& lhs, Json rhs)
    {
        lhs.swap(rhs);
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::array>
{
    static const bool is_assignable = true;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_array();
    }
    static void assign(Json& lhs, typename Json::array rhs)
    {
        lhs.assign_array(rhs);
    }
};

template<class Json>
struct json_type_traits<Json, jsoncons::null_type>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, bool>
{
    static const bool is_assignable = true;

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

template<class Json,class T>
struct json_type_traits<Json, T, typename std::enable_if<std::is_same<T, 
    std::conditional<!std::is_same<bool,std::vector<bool>::const_reference>::value,
                     std::vector<bool>::const_reference,
                     void>::type>::value>::type>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, std::vector<bool>::reference>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, short>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, unsigned short>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, int>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, unsigned int>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, long>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, unsigned long>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, long long>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, unsigned long long>
{
    static const bool is_assignable = true;

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
struct json_type_traits<Json, float>
{
    static const bool is_assignable = true;

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

template<class Json>
struct json_type_traits<Json, double>
{
    static const bool is_assignable = true;

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

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
std::is_void<typename json_string_type_traits<Json,T>::char_traits_type>::value && 
std::integral_constant<bool, json_type_traits<Json, typename T::iterator::value_type>::is_assignable &&
                             json_type_traits<Json, typename T::const_iterator::value_type>::is_assignable>::value
>::type>
{
    typedef typename T::iterator::value_type element_type;

    static const bool is_assignable = true;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_array();
        if (result)
        {
            for (auto e : rhs.elements())
            {
                if (!e.template is<element_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    static T as(const Json& rhs)
    {
        if (rhs.is_array())
        {
            T v(json_array_input_iterator<Json, element_type>(rhs.elements().begin()),
                json_array_input_iterator<Json, element_type>(rhs.elements().end()));
            return v;
        }
        else
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempt to cast json non-array to array");
        }
    }

    static void assign(Json& lhs, const T& rhs)
    {
        lhs = Json(std::begin(rhs), std::end(rhs));
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    !std::is_void<typename json_string_type_traits<Json,T>::char_traits_type>::value && 
std::integral_constant<bool, json_type_traits<Json, typename T::iterator::value_type>::is_assignable &&
                             json_type_traits<Json, typename T::const_iterator::value_type>::is_assignable>::value>::type>
{
    typedef typename T::iterator::value_type element_type;

    static const bool is_assignable = true;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        return rhs.is_string();
    }

    static T as(const Json& rhs)
    {
        return rhs.as_string();
    }

    static void assign(Json& lhs, const T& rhs)
    {
        lhs.assign_string(rhs);
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                       typename std::enable_if<std::integral_constant<bool, json_type_traits<Json, typename T::mapped_type>::is_assignable>::value>::type>
{
    typedef typename T::key_type key_type;
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;

    static const bool is_assignable = true;

    static bool is(const Json& rhs) JSONCONS_NOEXCEPT
    {
        bool result = rhs.is_object();
        for (auto member : rhs.members())
        {
            if (!member.value().template is<mapped_type>())
            {
                result = false;
            }
        }
        return result;
    }

    static T as(const Json& rhs)
    {
        T v(json_object_input_iterator<Json,value_type>(rhs.members().begin()),
            json_object_input_iterator<Json,value_type>(rhs.members().end()));
        return v;
    }

    static void assign(Json& lhs, const T& rhs)
    {
        Json val;
        val.reserve(rhs.size());
        for (auto p: rhs)
        {
            val.set(p.first,p.second);
        }
        lhs.swap(val);
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
