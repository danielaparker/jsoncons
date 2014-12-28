// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON2_HPP
#define JSONCONS_JSON2_HPP

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
#include "jsoncons/json_type_traits.hpp"
#include "jsoncons/json_structures.hpp"
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json_deserializer.hpp"
#include "jsoncons/json_serializer.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif


namespace jsoncons {

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json()
{
    type_ = empty_object_t;
}

template<typename Char, typename Alloc>
template<class InputIterator>
basic_json<Char, Alloc>::basic_json(InputIterator first, InputIterator last)
{
    type_ = array_t;
    value_.array_ = new json_array<Char, Alloc>(first, last);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(const basic_json<Char, Alloc>& val)
{
    type_ = val.type_;
    switch (type_)
    {
    case null_t:
    case empty_object_t:
        break;
    case double_t:
    case longlong_t:
    case ulonglong_t:
    case bool_t:
        value_ = val.value_;
        break;
    case string_t:
        value_.string_wrapper_ = new string_wrapper(val.internal_string());
        break;
    case array_t:
        value_.array_ = val.value_.array_->clone();
        break;
    case object_t:
        value_.object_ = val.value_.object_->clone();
        break;
    case json_any_t:
        value_.any_value_ = new any(*(val.value_.any_value_));
        break;
    default:
        // throw
        break;
    }
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(json_object<Char, Alloc> *var)
{
    type_ = object_t;
    value_.object_ = var;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(json_array<Char, Alloc> *var)
{
    type_ = array_t;
    value_.array_ = var;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(typename basic_json<Char, Alloc>::any var)
{
    type_ = json_any_t;
    value_.any_value_ = new any(var);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(jsoncons::null_type)
{
    type_ = null_t;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(double val)
{
    type_ = double_t;
    value_.double_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(long long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(int val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(unsigned int val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(unsigned long val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(unsigned long long val)
{
    type_ = ulonglong_t;
    value_.ulonglong_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(bool val)
{
    type_ = bool_t;
    value_.bool_value_ = val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(Char c)
{
    type_ = string_t;
    value_.string_wrapper_ = new string_wrapper(c);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(const std::basic_string<Char>& s)
{
    type_ = string_t;
    value_.string_wrapper_ = new string_wrapper(s);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(const Char *s)
{
    type_ = string_t;
    value_.string_wrapper_ = new string_wrapper(s);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(const Char *s, size_t length)
{
    type_ = string_t;
    value_.string_wrapper_ = new string_wrapper(s,length);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(value_type t)
{
    type_ = t;
    switch (type_)
    {
    case null_t:
    case empty_object_t:
    case double_t:
    case longlong_t:
    case ulonglong_t:
    case bool_t:
        break;
    case string_t:
        value_.string_wrapper_ = new string_wrapper();
        break;
    case array_t:
        value_.array_ = new json_array<Char, Alloc>();
        break;
    case object_t:
        value_.object_ = new json_object<Char, Alloc>();
        break;

    case json_any_t:
        JSONCONS_ASSERT(false);
    }
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::~basic_json()
{
    switch (type_)
    {
    case null_t:
    case empty_object_t:
    case double_t:
    case longlong_t:
    case ulonglong_t:
    case bool_t:
        break;
    case string_t:
        delete value_.string_wrapper_;
        break;
    case array_t:
        delete value_.array_;
        break;
    case object_t:
        delete value_.object_;
        break;
    case json_any_t:
        delete value_.any_value_;
        break;
    }
}

template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_any(const typename basic_json<Char,Alloc>::any& rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = json_any_t;
        value_.any_value_ = new any(rhs);
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}

template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_string(const std::basic_string<Char>& rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = string_t;
        value_.string_wrapper_ = new string_wrapper(rhs);
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}

template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_double(double rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = double_t;
        value_.double_value_ = rhs;
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}

template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_longlong(long long rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = longlong_t;
        value_.longlong_value_ = rhs;
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}
template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_ulonglong(unsigned long long rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = ulonglong_t;
        value_.ulonglong_value_ = rhs;
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}
template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_bool(bool rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = bool_t;
        value_.bool_value_ = rhs;
        break;
    default:
        basic_json<Char, Alloc>(rhs).swap(*this);
        break;
    }
}
template<typename Char, class Alloc>
void basic_json<Char, Alloc>::assign_null()
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
    case longlong_t:
    case ulonglong_t:
    case double_t:
        type_ = null_t;
        break;
    default:
        basic_json<Char, Alloc>(null_type()).swap(*this);
        break;
    }
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>& basic_json<Char, Alloc>::operator = (basic_json<Char, Alloc> rhs)
{
    rhs.swap(*this);
    return *this;
}

template<typename Char, typename Alloc>
bool basic_json<Char, Alloc>::operator!=(const basic_json<Char, Alloc>& rhs) const
{
    return !(*this == rhs);
}

template<typename Char, typename Alloc>
bool basic_json<Char, Alloc>::operator==(const basic_json<Char, Alloc>& rhs) const
{
    if (is_number() && rhs.is_number())
    {
        switch (type_)
        {
        case longlong_t:
            switch (rhs.type())
            {
            case longlong_t:
                return value_.longlong_value_ == rhs.value_.longlong_value_;
            case ulonglong_t:
                return value_.longlong_value_ == rhs.value_.ulonglong_value_;
            case double_t:
                return value_.longlong_value_ == rhs.value_.double_value_;
            }
            break;
        case ulonglong_t:
            switch (rhs.type())
            {
            case longlong_t:
                return value_.ulonglong_value_ == rhs.value_.longlong_value_;
            case ulonglong_t:
                return value_.ulonglong_value_ == rhs.value_.ulonglong_value_;
            case double_t:
                return value_.ulonglong_value_ == rhs.value_.double_value_;
            }
            break;
        case double_t:
            switch (rhs.type())
            {
            case longlong_t:
                return value_.double_value_ == rhs.value_.longlong_value_;
            case ulonglong_t:
                return value_.double_value_ == rhs.value_.ulonglong_value_;
            case double_t:
                return value_.double_value_ == rhs.value_.double_value_;
            }
            break;
        }
    }

    if (rhs.type_ != type_)
    {
        return false;
    }
    switch (type_)
    {
    case bool_t:
        return value_.bool_value_ == rhs.value_.bool_value_;
    case null_t:
    case empty_object_t:
        return true;
    case string_t:
        return internal_string() == rhs.internal_string();
    case array_t:
        return *(value_.array_) == *(rhs.value_.array_);
        break;
    case object_t:
        return *(value_.object_) == *(rhs.value_.object_);
        break;
    case json_any_t:
        break;
    default:
        // throw
        break;
    }
    return false;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>& basic_json<Char, Alloc>::at(size_t i)
{
    switch (type_)
    {
    case empty_object_t:
        JSONCONS_THROW_EXCEPTION("Out of range");
    case object_t:
        return value_.object_->at(i);
    case array_t:
        return value_.array_->at(i);
    default:
        JSONCONS_THROW_EXCEPTION("Not an array or object");
    }
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc>& basic_json<Char, Alloc>::at(size_t i) const
{
    switch (type_)
    {
    case empty_object_t:
        JSONCONS_THROW_EXCEPTION("Out of range");
    case object_t:
        return value_.object_->at(i);
    case array_t:
        return value_.array_->at(i);
    default:
        JSONCONS_THROW_EXCEPTION("Not an array or object");
    }
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>& basic_json<Char, Alloc>::at(const std::basic_string<Char>& name)
{
    switch (type_)
    {
    case empty_object_t:
        JSONCONS_THROW_EXCEPTION_1("%s not found", name);
    case object_t:
        return value_.object_->get(name);
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc>& basic_json<Char, Alloc>::at(const std::basic_string<Char>& name) const
{
    switch (type_)
    {
    case empty_object_t:
        JSONCONS_THROW_EXCEPTION_1("%s not found", name);
    case object_t:
        return value_.object_->get(name);
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc>& basic_json<Char, Alloc>::get(const std::basic_string<Char>& name) const
{
    switch (type_)
    {
    case empty_object_t:
        return basic_json<Char, Alloc>::null;
    case object_t:
        {
            const_object_iterator it = value_.object_->find(name);
            return it != end_members() ? it->value() : basic_json<Char, Alloc>::null;
        }
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
template<typename T>
typename basic_json<Char, Alloc>::const_val_proxy basic_json<Char, Alloc>::get(const std::basic_string<Char>& name, T default_val) const
{
    switch (type_)
    {
    case empty_object_t:
        {
            return const_val_proxy(default_val);
        }
    case object_t:
        {
            const_object_iterator it = value_.object_->find(name);
            if (it != end_members())
            {
                return const_val_proxy(it->value());
            }
            else
            {
                return const_val_proxy(default_val);
            }
        }
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::set(const std::basic_string<Char>& name, const basic_json<Char, Alloc>& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char, Alloc>();
    case object_t:
        value_.object_->set(name, value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>::basic_json(basic_json&& other){
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = null_t;
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::add(basic_json<Char, Alloc>&& value){
    switch (type_){
    case array_t:
        value_.array_->push_back(value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::add(size_t index, basic_json<Char, Alloc>&& value){
    switch (type_){
    case array_t:
        value_.array_->add(index, value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::set(std::basic_string<Char>&& name, basic_json<Char, Alloc>&& value){
    switch (type_){
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Alloc>();
    case object_t:
        value_.object_->set(name,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

// Deprecated
template<typename Char, typename Alloc>
template<class T>
void basic_json<Char, Alloc>::set_custom_data(const std::basic_string<Char>& name, T value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char, Alloc>();
    case object_t:
        value_.object_->set(name, basic_json<Char, Alloc>(any(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object", name);
        }
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::clear()
{
    switch (type_)
    {
    case array_t:
        value_.array_->clear();
        break;
    case object_t:
        value_.object_->clear();
        break;
    default:
        break;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::remove_range(size_t from_index, size_t to_index)
{
    switch (type_)
    {
    case array_t:
        value_.array_->remove_range(from_index, to_index);
        break;
    default:
        break;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::remove_member(const std::basic_string<Char>& name)
{
    switch (type_)
    {
    case object_t:
        value_.object_->remove(name);
        break;
    default:
        break;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::add(const basic_json<Char, Alloc>& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->push_back(value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::add(size_t index, const basic_json<Char, Alloc>& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index, value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

// Deprecated
template<typename Char, typename Alloc>
template<class T>
void basic_json<Char, Alloc>::add_custom_data(T value)
{
    switch (type_)
    {
    case array_t:
        {
            add(basic_json<Char, Alloc>(any(value)));
        }
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

// Deprecated
template<typename Char, typename Alloc>
template<class T>
void basic_json<Char, Alloc>::add_custom_data(size_t index, T value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index, basic_json<Char, Alloc>(any(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template<typename Char, typename Alloc>
size_t basic_json<Char, Alloc>::size() const
{
    switch (type_)
    {
    case empty_object_t:
        return 0;
    case object_t:
        return value_.object_->size();
    case array_t:
        return value_.array_->size();
    default:
        return 0;
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::object_key_proxy basic_json<Char, Alloc>::operator[](const std::basic_string<Char>& name)
{
    return object_key_proxy(*this, name);
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc>& basic_json<Char, Alloc>::operator[](const std::basic_string<Char>& name) const
{
    return at(name);
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc>& basic_json<Char, Alloc>::operator[](size_t i)
{
    return at(i);
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc>& basic_json<Char, Alloc>::operator[](size_t i) const
{
    return at(i);
}

template<typename Char, typename Alloc>
std::basic_string<Char> basic_json<Char, Alloc>::to_string() const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer);
    return os.str();
}

template<typename Char, typename Alloc>
std::basic_string<Char> basic_json<Char, Alloc>::to_string(const basic_output_format<Char>& format) const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os, format);
    to_stream(serializer);
    return os.str();
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::to_stream(basic_json_output_handler<Char>& handler) const
{
    switch (type_)
    {
    case string_t:
        handler.value(&internal_string()[0],internal_string().length());
        break;
    case double_t:
        handler.value(value_.double_value_);
        break;
    case longlong_t:
        handler.value(value_.longlong_value_);
        break;
    case ulonglong_t:
        handler.value(value_.ulonglong_value_);
        break;
    case bool_t:
        handler.value(value_.bool_value_);
        break;
    case null_t:
        handler.value(null_type());
        break;
    case empty_object_t:
        handler.begin_object();
        handler.end_object();
        break;
    case object_t:
        {
            handler.begin_object();
            json_object<Char, Alloc> *o = value_.object_;
            for (const_object_iterator it = o->begin(); it != o->end(); ++it)
            {
                handler.name(&(it->name())[0],it->name().length());
                it->value().to_stream(handler);
            }
            handler.end_object();
        }
        break;
    case array_t:
        {
            handler.begin_array();
            json_array<Char, Alloc> *o = value_.array_;
            for (const_array_iterator it = o->begin(); it != o->end(); ++it)
            {
                it->to_stream(handler);
            }
            handler.end_array();
        }
        break;
    case json_any_t:
        value_.any_value_->to_stream(handler);
        break;
    default:
        // throw
        break;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::to_stream(std::basic_ostream<Char>& os) const
{
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer);
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
{
    basic_json_serializer<Char> serializer(os, format);
    to_stream(serializer);
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const
{
    basic_json_serializer<Char> serializer(os, format, indenting);
    to_stream(serializer);
}

template<typename Char, typename Alloc>
const basic_json<Char, Alloc> basic_json<Char, Alloc>::an_object(new json_object<Char, Alloc>());

template<typename Char, typename Alloc>
const basic_json<Char, Alloc> basic_json<Char, Alloc>::an_array(new json_array<Char, Alloc>());

template<typename Char, typename Alloc>
const basic_json<Char, Alloc> basic_json<Char, Alloc>::null = basic_json<Char, Alloc>(jsoncons::null_type());


/*template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_array()
{
    return basic_json<Char, Alloc>(new json_array<Char, Alloc>());
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_array(size_t n)
{
    return basic_json<Char, Alloc>(new json_array<Char, Alloc>(n));
}

template<typename Char, typename Alloc>
template<typename T>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_array(size_t n, T val)
{
    basic_json<Char, Alloc> v;
    v = val;
    return basic_json<Char, Alloc>(new json_array<Char, Alloc>(n, v));
}*/

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_2d_array(size_t m, size_t n)
{
    basic_json<Char, Alloc> a(basic_json<Char, Alloc>(new json_array<Char, Alloc>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char, Alloc>::make_array(n);
    }
    return a;
}

template<typename Char, typename Alloc>
template<typename T>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_2d_array(size_t m, size_t n, T val)
{
    basic_json<Char, Alloc> v;
    v = val;
    basic_json<Char, Alloc> a(basic_json<Char, Alloc>(new json_array<Char, Alloc>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char, Alloc>::make_array(n, v);
    }
    return a;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<Char, Alloc> a(basic_json<Char, Alloc>(new json_array<Char, Alloc>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char, Alloc>::make_2d_array(n, k);
    }
    return a;
}

template<typename Char, typename Alloc>
template<typename T>
basic_json<Char, Alloc> basic_json<Char, Alloc>::make_3d_array(size_t m, size_t n, size_t k, T val)
{
    basic_json<Char, Alloc> v;
    v = val;
    basic_json<Char, Alloc> a(basic_json<Char, Alloc>(new json_array<Char, Alloc>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char, Alloc>::make_2d_array(n, k, v);
    }
    return a;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse(std::basic_istream<Char>& is)
{
    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse(std::basic_istream<Char>& is, 
                                                       basic_parse_error_handler<Char>& err_handler)
{
    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler, err_handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse_string(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse_string(const std::basic_string<Char>& s, 
                                                              basic_parse_error_handler<Char>& err_handler)
{
    std::basic_istringstream<Char> is(s);
    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler, err_handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse_file(const std::string& filename)
{
    std::basic_ifstream<Char> is(filename.c_str(), std::basic_ifstream<Char>::in | std::basic_ifstream<Char>::binary);
    if (!is.is_open())
    {
        throw json_exception_1<char>("Cannot open file %s", filename);
    }

    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
basic_json<Char, Alloc> basic_json<Char, Alloc>::parse_file(const std::string& filename, 
                                                            basic_parse_error_handler<Char>& err_handler)
{
    std::basic_ifstream<Char> is(filename.c_str(), std::basic_ifstream<Char>::in | std::basic_ifstream<Char>::binary);
    if (!is.is_open())
    {
        throw json_exception_1<char>("Cannot open file %s", filename);
    }

    basic_json_deserializer<Char, Alloc> handler;
    basic_json_reader<Char> reader(is, handler, err_handler);
    reader.read();
    basic_json<Char, Alloc> val;
    handler.root().swap(val);
    return val;
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::object_iterator basic_json<Char, Alloc>::begin_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char, Alloc>();
    case object_t:
        return value_.object_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::const_object_iterator basic_json<Char, Alloc>::begin_members() const
{
    switch (type_)
    {
    case empty_object_t:
        return an_object.begin_members();
    case object_t:
        return value_.object_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::object_iterator basic_json<Char, Alloc>::end_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char, Alloc>();
    case object_t:
        return value_.object_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::const_object_iterator basic_json<Char, Alloc>::end_members() const
{
    switch (type_)
    {
    case empty_object_t:
        return an_object.end_members();
    case object_t:
        return value_.object_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::array_iterator basic_json<Char, Alloc>::begin_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::const_array_iterator basic_json<Char, Alloc>::begin_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::array_iterator basic_json<Char, Alloc>::end_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template<typename Char, typename Alloc>
template <class T>
basic_json<Char, Alloc>& basic_json<Char, Alloc>::operator=(T val)
{
    value_adapter<Char,Alloc,T> adapter;
    adapter.assign(*this,val);
    return *this;
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::const_array_iterator basic_json<Char, Alloc>::end_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template<typename Char, typename Alloc>
double basic_json<Char, Alloc>::as_double() const
{
    switch (type_)
    {
    case double_t:
        return value_.double_value_;
    case longlong_t:
        return static_cast<double>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<double>(value_.ulonglong_value_);
    case null_t:
        return std::numeric_limits<double>::quiet_NaN();;
    default:
        JSONCONS_THROW_EXCEPTION("Not a double");
    }
}

template<typename Char, typename Alloc>
bool basic_json<Char, Alloc>::is_empty() const
{
    switch (type_)
    {
    case string_t:
        return internal_string().size() == 0;
    case array_t:
        return value_.array_->size() == 0;
    case empty_object_t:
        return true;
    case object_t:
        return value_.object_->size() == 0;
    default:
        return false;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::reserve(size_t n)
{
    switch (type_)
    {
    case array_t:
        value_.array_->reserve(n);
        break;
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char, Alloc>();
    case object_t:
        value_.object_->reserve(n);
        break;
    }
}

template<typename Char, typename Alloc>
void basic_json<Char, Alloc>::resize_array(size_t n)
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n);
        break;
    }
}

template<typename Char, typename Alloc>
template<typename T>
void basic_json<Char, Alloc>::resize_array(size_t n, T val)
{
    basic_json<Char, Alloc> j;
    j = val;
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n, j);
        break;
    }
}

template<typename Char, typename Alloc>
size_t basic_json<Char, Alloc>::capacity() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->capacity();
    case object_t:
        return value_.object_->capacity();
    default:
        return 0;
    }
}

template<typename Char, typename Alloc>
bool basic_json<Char, Alloc>::has_member(const std::basic_string<Char>& name) const
{
    switch (type_)
    {
    case object_t:
        {
            const_object_iterator it = value_.object_->find(name);
            return it != end_members();
        }
        break;
    default:
        return false;
    }
}

template<typename Char, typename Alloc>
bool basic_json<Char, Alloc>::as_bool() const
{
    switch (type_)
    {
    case bool_t:
        return value_.bool_value_;
    default:
        JSONCONS_THROW_EXCEPTION("Not a bool");
    }
}

template<typename Char, typename Alloc>
int basic_json<Char, Alloc>::as_int() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<int>(value_.double_value_);
    case longlong_t:
        return static_cast<int>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<int>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not a int");
    }
}

template<typename Char, typename Alloc>
unsigned int basic_json<Char, Alloc>::as_uint() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned int>(value_.double_value_);
    case longlong_t:
        return static_cast<unsigned int>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<unsigned int>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not a unsigned int");
    }
}

template<typename Char, typename Alloc>
long long basic_json<Char, Alloc>::as_longlong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<long long>(value_.double_value_);
    case longlong_t:
        return static_cast<long long>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<long long>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not a long long");
    }
}

template<typename Char, typename Alloc>
unsigned long long basic_json<Char, Alloc>::as_ulonglong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned long long>(value_.double_value_);
    case longlong_t:
        return static_cast<unsigned long long>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<unsigned long long>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not a unsigned long long");
    }
}

template<typename Char, typename Alloc>
long basic_json<Char, Alloc>::as_long() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<long long>(value_.double_value_);
    case longlong_t:
        return static_cast<long long>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<long long>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not a long");
    }
}

template<typename Char, typename Alloc>
unsigned long basic_json<Char, Alloc>::as_ulong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned long long>(value_.double_value_);
    case longlong_t:
        return static_cast<unsigned long long>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<unsigned long long>(value_.ulonglong_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    default:
        JSONCONS_THROW_EXCEPTION("Not an unsigned long");
    }
}

// Deprecated
template<typename Char, typename Alloc>
template<class T>
const T& basic_json<Char, Alloc>::custom_data() const
{
    switch (type_)
    {
    case json_any_t:
			const T& p = value_.any_value_->template cast<T>();
			return p;
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

// Deprecated
template<typename Char, typename Alloc>
template<class T>
T& basic_json<Char, Alloc>::custom_data()
{
    switch (type_)
    {
    case json_any_t:
        {
			T& p = value_.any_value_->template cast<T>();
			return p;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::any& basic_json<Char, Alloc>::any_value()
{
    switch (type_)
    {
    case json_any_t:
        {
			return *value_.any_value_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an any value");
    }
}

template<typename Char, typename Alloc>
const typename basic_json<Char, Alloc>::any& basic_json<Char, Alloc>::any_value() const
{
    switch (type_)
    {
    case json_any_t:
        {
			return *value_.any_value_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an any value");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::object& basic_json<Char, Alloc>::object_value()
{
    switch (type_)
    {
    case object_t:
        {
			return *value_.object_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an object value");
    }
}

template<typename Char, typename Alloc>
const typename basic_json<Char, Alloc>::object& basic_json<Char, Alloc>::object_value() const
{
    switch (type_)
    {
    case object_t:
        {
			return *value_.object_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an object value");
    }
}

template<typename Char, typename Alloc>
typename basic_json<Char, Alloc>::array& basic_json<Char, Alloc>::array_value()
{
    switch (type_)
    {
    case array_t:
        {
			return *value_.array_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an array value");
    }
}

template<typename Char, typename Alloc>
const typename basic_json<Char, Alloc>::array& basic_json<Char, Alloc>::array_value() const
{
    switch (type_)
    {
    case array_t:
        {
			return *value_.array_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not an array value");
    }
}

template<typename Char, typename Alloc>
std::basic_string<Char> basic_json<Char, Alloc>::as_string() const
{
    switch (type_)
    {
    case string_t:
        return std::basic_string<Char>(&internal_string()[0],internal_string().length());
    default:
        return to_string();
    }
}

template<typename Char, typename Alloc>
std::basic_string<Char> basic_json<Char, Alloc>::as_string(const basic_output_format<Char>& format) const
{
    switch (type_)
    {
    case string_t:
        return std::basic_string<Char>(&internal_string()[0],internal_string().length());
    default:
        return to_string(format);
    }
}

template<typename Char, typename Alloc>
Char basic_json<Char, Alloc>::as_char() const
{
    switch (type_)
    {
    case string_t:
        return internal_string().length() > 0 ? internal_string()[0] : '\0';
    case longlong_t:
        return static_cast<Char>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<Char>(value_.ulonglong_value_);
    case double_t:
        return static_cast<Char>(value_.double_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    case null_t:
        return 0;
    default:
        JSONCONS_THROW_EXCEPTION("Cannot convert to char");
    }
}

template<typename Char, typename Alloc>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const basic_json<Char, Alloc>& o)
{
    o.to_stream(os);
    return os;
}

template<typename Char, typename Alloc>
class pretty_printer
{
public:
    pretty_printer(const basic_json<Char, Alloc>& o)
       : o_(&o)
    {
    }

    pretty_printer(const basic_json<Char, Alloc>& o,
                   const basic_output_format<Char>& format)
       : o_(&o), format_(format)
    {
        ;
    }

    void to_stream(std::basic_ostream<Char>& os) const
    {
        o_->to_stream(os, format_, true);
    }

    friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const pretty_printer<Char, Alloc>& o)
    {
        o.to_stream(os);
        return os;
    }

    const basic_json<Char, Alloc> *o_;
    basic_output_format<Char> format_;
private:
    pretty_printer();
};

template<typename Char, class Alloc>
pretty_printer<Char, Alloc> pretty_print(const basic_json<Char, Alloc>& val)
{
    return pretty_printer<Char, Alloc>(val);
}

template<typename Char, class Alloc>
pretty_printer<Char, Alloc> pretty_print(const basic_json<Char, Alloc>& val,
                                           const basic_output_format<Char>& format)
{
    return pretty_printer<Char, Alloc>(val, format);
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

template<typename Char>
void escape_string(const Char* s,
                   size_t length,
                   const basic_output_format<Char>& format,
                   std::basic_ostream<Char>& os)
{
    const Char* begin = s;
    const Char* end = s + length;
    for (const Char* it = begin; it != end; ++it)
    {
        Char c = *it;
        switch (c)
        {
        case '\\':
            os << '\\' << '\\';
            break;
        case '"':
            os << '\\' << '\"';
            break;
        case '\b':
            os << '\\' << 'b';
            break;
        case '\f':
            os << '\\';
            os << 'f';
            break;
        case '\n':
            os << '\\';
            os << 'n';
            break;
        case '\r':
            os << '\\';
            os << 'r';
            break;
        case '\t':
            os << '\\';
            os << 't';
            break;
        default:
            uint32_t u(c >= 0 ? c : 256 + c);
            if (format.escape_solidus() && c == '/')
            {
                os << '\\';
                os << '/';
            }
            else if (is_control_character(u) || format.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                uint32_t cp = json_char_traits<Char, sizeof(Char)>::convert_char_to_codepoint(it, end);
                if (is_non_ascii_character(cp) || is_control_character(u))
                {
                    if (cp > 0xFFFF)
                    {
                        cp -= 0x10000;
                        uint32_t first = (cp >> 10) + 0xD800;
                        uint32_t second = ((cp & 0x03FF) + 0xDC00);

                        os << '\\';
                        os << 'u';
                        os << to_hex_character(first >> 12 & 0x000F);
                        os << to_hex_character(first >> 8  & 0x000F);
                        os << to_hex_character(first >> 4  & 0x000F);
                        os << to_hex_character(first     & 0x000F);
                        os << '\\';
                        os << 'u';
                        os << to_hex_character(second >> 12 & 0x000F);
                        os << to_hex_character(second >> 8  & 0x000F);
                        os << to_hex_character(second >> 4  & 0x000F);
                        os << to_hex_character(second     & 0x000F);
                    }
                    else
                    {
                        os << '\\';
                        os << 'u';
                        os << to_hex_character(cp >> 12 & 0x000F);
                        os << to_hex_character(cp >> 8  & 0x000F);
                        os << to_hex_character(cp >> 4  & 0x000F);
                        os << to_hex_character(cp     & 0x000F);
                    }
                }
                else
                {
                    os << c;
                }
            }
            else if (format.escape_solidus() && c == '/')
            {
                os << '\\';
                os << '/';
            }
            else
            {
                os << c;
            }
            break;
        }
    }
}

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
