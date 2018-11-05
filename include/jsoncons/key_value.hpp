// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_KEYVALUE_HPP
#define JSONCONS_JSON_KEYVALUE_HPP

#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>

namespace jsoncons {

template <class KeyT, class ValueT>
class key_value
{
public:
    typedef KeyT key_type;
    typedef ValueT value_type;
    typedef typename KeyT::allocator_type allocator_type;
    typedef typename value_type::string_view_type string_view_type;
private:
    key_type key_;
    value_type value_;
public:

    key_value()
    {
    }

    key_value(const key_type& name, const value_type& val)
        : key_(name), value_(val)
    {
    }

    key_value(const string_view_type& name)
        : key_(name)
    {
    }

    template <class T>
    key_value(key_type&& name, T&& val)
        : key_(std::forward<key_type>(name)), 
          value_(std::forward<T>(val))
    {
    }

    template <class T>
    key_value(key_type&& name, 
                   T&& val, 
                   const allocator_type& allocator)
        : key_(std::forward<key_type>(name)), value_(std::forward<T>(val), allocator)
    {
    }

    key_value(const key_value& member)
        : key_(member.key_), value_(member.value_)
    {
    }

    key_value(key_value&& member)
        : key_(std::move(member.key_)), value_(std::move(member.value_))
    {
    }

    string_view_type key() const
    {
        return string_view_type(key_.data(),key_.size());
    }

    value_type& value()
    {
        return value_;
    }

    const value_type& value() const
    {
        return value_;
    }

    template <class T>
    void value(T&& value)
    {
        value_ = std::forward<T>(value);
    }

    void swap(key_value& member)
    {
        key_.swap(member.key_);
        value_.swap(member.value_);
    }

    key_value& operator=(const key_value& member)
    {
        if (this != & member)
        {
            key_ = member.key_;
            value_ = member.value_;
        }
        return *this;
    }

    key_value& operator=(key_value&& member)
    {
        if (this != &member)
        {
            key_.swap(member.key_);
            value_.swap(member.value_);
        }
        return *this;
    }

    void shrink_to_fit() 
    {
        key_.shrink_to_fit();
        value_.shrink_to_fit();
    }
#if !defined(JSONCONS_NO_DEPRECATED)
    const key_type& name() const
    {
        return key_;
    }
#endif
};

}

#endif
