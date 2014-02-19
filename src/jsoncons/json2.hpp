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
#include "jsoncons/json_structures.hpp"
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json_deserializer.hpp"
#include "jsoncons/json_serializer.hpp"


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif


namespace jsoncons {

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json()
{
    type_ = empty_object_t;
}

template <typename Char, typename Allocator>
template <class InputIterator>
basic_json<Char,Allocator>::basic_json(InputIterator first, InputIterator last)
{
    type_ = array_t;
    value_.array_ = new json_array<Char,Allocator>(first,last);
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(const basic_json<Char,Allocator>& val)
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
        value_.value_string_ = new std::basic_string<Char>(*(val.value_.value_string_));
        break;
    case array_t:
        value_.array_ = val.value_.array_->clone();
        break;
    case object_t:
        value_.object_ = val.value_.object_->clone();
        break;
    case custom_t:
        value_.userdata_ = val.value_.userdata_->clone();
        break;
    default:
        // throw
        break;
    }
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(json_object<Char,Allocator>* var)
{
    type_ = object_t;
    value_.object_ = var;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(json_array<Char,Allocator>* var)
{
    type_ = array_t;
    value_.array_ = var;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(basic_custom_data<Char>* var)
{
    type_ = custom_t;
    value_.userdata_ = var;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(double val)
{
    type_ = double_t;
    value_.value_double_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(long long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(int val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(unsigned int val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(unsigned long val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(unsigned long long val)
{
    type_ = ulonglong_t;
    value_.ulonglong_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(bool val)
{
    type_ = bool_t;
    value_.bool_value_ = val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(Char c)
{
    type_ = string_t;
    value_.value_string_ = new std::basic_string<Char>();
    value_.value_string_->push_back(c);
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(const std::basic_string<Char>& s)
{
    type_ = string_t;
    value_.value_string_ = new std::basic_string<Char>(s);
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(const Char* s)
{
    type_ = string_t;
    value_.value_string_ = new std::basic_string<Char>(s);
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(value_type t)
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
        value_.value_string_ = new std::basic_string<Char>();
        break;
    case array_t:
        value_.array_ = new json_array<Char,Allocator>();
        break;
    case object_t:
        value_.object_ = new json_object<Char,Allocator>();
        break;

    case custom_t:
        JSONCONS_ASSERT(false);
    }
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::~basic_json()
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
        delete value_.value_string_;
        break;
    case array_t:
        delete value_.array_;
        break;
    case object_t:
        delete value_.object_;
        break;
    case custom_t:
        delete value_.userdata_;
        break;
    }
}
template <class Char,class Allocator>
void basic_json<Char,Allocator>::assign_string(const std::basic_string<Char>& rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = string_t;
        value_.value_string_ = new std::basic_string<Char>(rhs);
        break;
    default:
        basic_json<Char,Allocator>(rhs).swap(*this);
        break;
    }
}
template <class Char,class Allocator>
void basic_json<Char,Allocator>::assign_double(double rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = double_t;
        value_.value_double_ = rhs;
        break;
    default:
        basic_json<Char,Allocator>(rhs).swap(*this);
        break;
    }
}

template <class Char,class Allocator>
void basic_json<Char,Allocator>::assign_longlong(long long rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = longlong_t;
        value_.longlong_value_ = rhs;
        break;
    default:
        basic_json<Char,Allocator>(rhs).swap(*this);
        break;
    }
}
template <class Char,class Allocator>
void basic_json<Char,Allocator>::assign_ulonglong(unsigned long long rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = ulonglong_t;
        value_.ulonglong_value_ = rhs;
        break;
    default:
        basic_json<Char,Allocator>(rhs).swap(*this);
        break;
    }
}
template <class Char,class Allocator>
void basic_json<Char,Allocator>::assign_bool(bool rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = bool_t;
        value_.bool_value_ = rhs;
        break;
    default:
        basic_json<Char,Allocator>(rhs).swap(*this);
        break;
    }
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>& basic_json<Char,Allocator>::operator=(basic_json<Char,Allocator> rhs)
{
    rhs.swap(*this);
    return *this;
}

template <typename Char, typename Allocator>
bool basic_json<Char,Allocator>::operator!=(const basic_json<Char,Allocator>& rhs) const
{
    return !(*this == rhs);
}

template <typename Char, typename Allocator>
bool basic_json<Char,Allocator>::operator==(const basic_json<Char,Allocator>& rhs) const
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
                return value_.longlong_value_ == rhs.value_.value_double_;
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
                return value_.ulonglong_value_ == rhs.value_.value_double_;
            }
            break;
        case double_t:
            switch (rhs.type())
            {
            case longlong_t:
                return value_.value_double_ == rhs.value_.longlong_value_;
            case ulonglong_t:
                return value_.value_double_ == rhs.value_.ulonglong_value_;
            case double_t:
                return value_.value_double_ == rhs.value_.value_double_;
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
        return *(value_.value_string_) == *(rhs.value_.value_string_);
    case array_t:
        return *(value_.array_) == *(rhs.value_.array_);
        break;
    case object_t:
        return *(value_.object_) == *(rhs.value_.object_);
        break;
    case custom_t:
        break;
    default:
        // throw
        break;
    }
    return false;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>& basic_json<Char,Allocator>::at(size_t i)
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

template <typename Char, typename Allocator>
const basic_json<Char,Allocator>& basic_json<Char,Allocator>::at(size_t i) const
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

template <typename Char, typename Allocator>
basic_json<Char,Allocator>& basic_json<Char,Allocator>::at(const std::basic_string<Char>& name)
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

template <typename Char, typename Allocator>
const basic_json<Char,Allocator>& basic_json<Char,Allocator>::at(const std::basic_string<Char>& name) const
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

template <typename Char, typename Allocator>
const basic_json<Char,Allocator>& basic_json<Char,Allocator>::get(const std::basic_string<Char>& name) const
{
    switch (type_)
    {
    case empty_object_t:
        return basic_json<Char,Allocator>::null;
    case object_t:
        {
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? it->value() : basic_json<Char,Allocator>::null;
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::const_val_proxy basic_json<Char,Allocator>::get(const std::basic_string<Char>& name, const basic_json<Char,Allocator>& default_val) const
{
    switch (type_)
    {
    case empty_object_t:
        return const_val_proxy(default_val);
    case object_t:
        {
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? const_val_proxy(it->value()) : const_val_proxy(default_val);
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::set(const std::basic_string<Char>& name, const basic_json<Char,Allocator>& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        value_.object_->set(name,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>::basic_json(basic_json&& other)
{
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = null_t;
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::add(basic_json<Char,Allocator>&& value)
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::add(size_t index, basic_json<Char,Allocator>&& value)
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::set(std::basic_string<Char>&& name, basic_json<Char,Allocator>&& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        value_.object_->set(name,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

template <typename Char, typename Allocator>
template <class T>
void basic_json<Char,Allocator>::set_custom_data(const std::basic_string<Char>& name, const T& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        value_.object_->set(name,basic_json<Char,Allocator>(new custom_data_wrapper<Char,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::clear()
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::remove_range(size_t from_index, size_t to_index)
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::remove_member(const std::basic_string<Char>& name)
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::add(const basic_json<Char,Allocator>& value)
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

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::add(size_t index, const basic_json<Char,Allocator>& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <typename Char, typename Allocator>
template <class T>
void basic_json<Char,Allocator>::add_custom_data(const T& value)
{
    switch (type_)
    {
    case array_t:
        {
        add(basic_json<Char,Allocator>(new custom_data_wrapper<Char,T>(value)));
        }
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <typename Char, typename Allocator>
template <class T>
void basic_json<Char,Allocator>::add_custom_data(size_t index, const T& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index,basic_json<Char,Allocator>(new custom_data_wrapper<Char,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <typename Char, typename Allocator>
size_t basic_json<Char,Allocator>::size() const
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

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::object_key_proxy basic_json<Char,Allocator>::operator[](const std::basic_string<Char>& name)
{
    return object_key_proxy(*this,name);
}

template <typename Char, typename Allocator>
const basic_json<Char,Allocator>& basic_json<Char,Allocator>::operator[](const std::basic_string<Char>& name) const
{
    return at(name);
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator>& basic_json<Char,Allocator>::operator[](size_t i)
{
    return value_.array_->at(i);
}

template <typename Char, typename Allocator>
const basic_json<Char,Allocator>& basic_json<Char,Allocator>::operator[](size_t i) const
{
    return value_.array_->at(i);
}

template <typename Char, typename Allocator>
std::basic_string<Char> basic_json<Char,Allocator>::to_string() const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer);
    return os.str();
}

template <typename Char, typename Allocator>
std::basic_string<Char> basic_json<Char,Allocator>::to_string(const basic_output_format<Char>& format) const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os,format);
    to_stream(serializer);
    return os.str();
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::to_stream(basic_json_output_handler<Char>& handler) const
{
    switch (type_)
    {
    case string_t:
        handler.value(*(value_.value_string_));
        break;
    case double_t:
        handler.value(value_.value_double_);
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
        handler.null_value();
        break;
    case empty_object_t:
        handler.begin_object();
        handler.end_object();
        break;
    case object_t:
        {
        handler.begin_object();
        json_object<Char,Allocator>* o = value_.object_;
        for (const_object_iterator it = o->begin(); it != o->end(); ++it)
        {
            handler.name(it->name());
            it->value().to_stream(handler);
        }
        handler.end_object();
        }
        break;
    case array_t:
        {
        handler.begin_array();
        json_array<Char,Allocator>* o = value_.array_;
        for (const_array_iterator it = o->begin(); it != o->end(); ++it)
        {
            it->to_stream(handler);
        }
        handler.end_array();
        }
        break;
    case custom_t:
        value_.userdata_->to_stream(handler);
        break;
    default:
        // throw
        break;
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::to_stream(std::basic_ostream<Char>& os) const
{
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer);
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
{
    basic_json_serializer<Char> serializer(os,format);
    to_stream(serializer);
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const
{
    basic_json_serializer<Char> serializer(os,format,indenting);
    to_stream(serializer);
}

template <typename Char, typename Allocator>
const basic_json<Char,Allocator> basic_json<Char,Allocator>::an_object(new json_object<Char,Allocator>());

template <typename Char, typename Allocator>
const basic_json<Char,Allocator> basic_json<Char,Allocator>::an_array(new json_array<Char,Allocator>());

template <typename Char, typename Allocator>
const basic_json<Char,Allocator> basic_json<Char,Allocator>::null = basic_json<Char,Allocator>(basic_json<Char,Allocator>::null_t);

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_array()
{
    return basic_json<Char,Allocator>(new json_array<Char,Allocator>());
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_array(size_t n)
{
    return basic_json<Char,Allocator>(new json_array<Char,Allocator>(n));
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_array(size_t n, const basic_json<Char,Allocator>& val)
{
    return basic_json<Char,Allocator>(new json_array<Char,Allocator>(n,val));
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_2d_array(size_t m, size_t n)
{
    basic_json<Char,Allocator> a(basic_json<Char,Allocator>(new json_array<Char,Allocator>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char,Allocator>::make_array(n);
    }
    return a;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_2d_array(size_t m, size_t n, const basic_json<Char,Allocator>& val)
{
    basic_json<Char,Allocator> a(basic_json<Char,Allocator>(new json_array<Char,Allocator>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char,Allocator>::make_array(n,val);
    }
    return a;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<Char,Allocator> a(basic_json<Char,Allocator>(new json_array<Char,Allocator>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char,Allocator>::make_2d_array(n,k);
    }
    return a;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::make_3d_array(size_t m, size_t n, size_t k, const basic_json<Char,Allocator>& val)
{
    basic_json<Char,Allocator> a(basic_json<Char,Allocator>(new json_array<Char,Allocator>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char,Allocator>::make_2d_array(n,k,val);
    }
    return a;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::parse(std::basic_istream<Char>& is)
{
    basic_json_deserializer<Char,Allocator> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char,Allocator> val;
    handler.root().swap(val);
    return val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::parse_string(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    basic_json_deserializer<Char,Allocator> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char,Allocator> val;
    handler.root().swap(val);
    return val;
}

template <typename Char, typename Allocator>
basic_json<Char,Allocator> basic_json<Char,Allocator>::parse_file(const std::string& filename)
{
    std::basic_ifstream<Char> is(filename.c_str(), std::basic_ifstream<Char>::in | std::basic_ifstream<Char>::binary);
    if (!is.is_open())
    {
        throw json_exception_1<char>("Cannot open file %s", filename);
    }

    basic_json_deserializer<Char,Allocator> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char,Allocator> val;
    handler.root().swap(val);
    return val;
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::object_iterator basic_json<Char,Allocator>::begin_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        return value_.object_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::const_object_iterator basic_json<Char,Allocator>::begin_members() const
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

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::object_iterator basic_json<Char,Allocator>::end_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        return value_.object_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::const_object_iterator basic_json<Char,Allocator>::end_members() const
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

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::array_iterator basic_json<Char,Allocator>::begin_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::const_array_iterator basic_json<Char,Allocator>::begin_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::array_iterator basic_json<Char,Allocator>::end_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <typename Char, typename Allocator>
typename basic_json<Char,Allocator>::const_array_iterator basic_json<Char,Allocator>::end_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <typename Char, typename Allocator>
double basic_json<Char,Allocator>::as_double() const
{
    switch (type_)
    {
    case double_t:
        return value_.value_double_;
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

template <typename Char, typename Allocator>
bool basic_json<Char,Allocator>::is_empty() const
{
    switch (type_)
    {
    case string_t:
        return value_.value_string_->size() == 0;
    case array_t:
        return value_.array_->size() == 0;
    case empty_object_t:
        return 0;
    case object_t:
        return value_.object_->size() == 0;
    default:
        return false;
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::reserve(size_t n)
{
    switch (type_)
    {
    case array_t:
        value_.array_->reserve(n);
        break;
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char,Allocator>();
    case object_t:
        value_.object_->reserve(n);
        break;
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::resize_array(size_t n)
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n);
        break;
    }
}

template <typename Char, typename Allocator>
void basic_json<Char,Allocator>::resize_array(size_t n, const basic_json<Char,Allocator>& val)
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n,val);
        break;
    }
}

template <typename Char, typename Allocator>
size_t basic_json<Char,Allocator>::capacity() const
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

template <typename Char, typename Allocator>
bool basic_json<Char,Allocator>::has_member(const std::basic_string<Char>& name) const
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

template <typename Char, typename Allocator>
bool basic_json<Char,Allocator>::as_bool() const
{
    switch (type_)
    {
    case bool_t:
        return value_.bool_value_;
    default:
        JSONCONS_THROW_EXCEPTION("Not a int");
    }
}

template <typename Char, typename Allocator>
int basic_json<Char,Allocator>::as_int() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<int>(value_.value_double_);
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

template <typename Char, typename Allocator>
unsigned int basic_json<Char,Allocator>::as_uint() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned int>(value_.value_double_);
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

template <typename Char, typename Allocator>
long long basic_json<Char,Allocator>::as_longlong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<long long>(value_.value_double_);
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

template <typename Char, typename Allocator>
unsigned long long basic_json<Char,Allocator>::as_ulonglong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned long long>(value_.value_double_);
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

template <typename Char, typename Allocator>
long basic_json<Char,Allocator>::as_long() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<long long>(value_.value_double_);
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

template <typename Char, typename Allocator>
unsigned long basic_json<Char,Allocator>::as_ulong() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<unsigned long long>(value_.value_double_);
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

template <typename Char, typename Allocator>
template <class T>
const T& basic_json<Char,Allocator>::custom_data() const
{
    switch (type_)
    {
    case custom_t:
        return static_cast<const custom_data_wrapper<Char,T>*>(value_.userdata_)->data1_;
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template <typename Char, typename Allocator>
template <class T>
T& basic_json<Char,Allocator>::custom_data()
{
    switch (type_)
    {
    case custom_t:
        {
        custom_data_wrapper<Char,T>* p = static_cast<custom_data_wrapper<Char,T>*>(value_.userdata_);
        return p->data1_;
        }
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template <typename Char, typename Allocator>
std::basic_string<Char> basic_json<Char,Allocator>::as_string() const
{
    switch (type_)
    {
    case string_t:
        return *(value_.value_string_);
    default:
        return to_string();
    }
}

template <typename Char, typename Allocator>
std::basic_string<Char> basic_json<Char,Allocator>::as_string(const basic_output_format<Char>& format) const
{
    switch (type_)
    {
    case string_t:
        return *(value_.value_string_);
    default:
        return to_string(format);
    }
}

template <typename Char, typename Allocator>
Char basic_json<Char,Allocator>::as_char() const
{
    switch (type_)
    {
    case string_t:
        return value_.value_string_->length() > 0 ? (*value_.value_string_)[0] : '\0';
    case longlong_t:
        return static_cast<Char>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<Char>(value_.ulonglong_value_);
    case double_t:
        return static_cast<Char>(value_.value_double_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    case null_t:
        return 0;
    default:
        JSONCONS_THROW_EXCEPTION("Cannot convert to char");
    }
}

template <typename Char, typename Allocator>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const basic_json<Char,Allocator>& o)
{
    o.to_stream(os);
    return os;
}

template <typename Char, typename Allocator>
class pretty_printer
{
public:
    pretty_printer(const basic_json<Char,Allocator>& o)
        : o_(&o)
    {
    }

    pretty_printer(basic_json<Char,Allocator>& o,
                   const basic_output_format<Char>& format)
        : o_(&o), format_(format)
    {
    ;
    }

    void to_stream(std::basic_ostream<Char>& os) const
    {
        o_->to_stream(os,format_,true);
    }

    friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const pretty_printer<Char,Allocator>& o)
    {
        o.to_stream(os);
        return os;
    }

    const basic_json<Char,Allocator>* o_;
    basic_output_format<Char> format_;
private:
    pretty_printer();
};

template <class Char,class Allocator>
pretty_printer<Char,Allocator> pretty_print(const basic_json<Char,Allocator>& val)
{
    return typename pretty_printer<Char,Allocator>(val);
}

template <class Char,class Allocator>
pretty_printer<Char,Allocator> pretty_print(const basic_json<Char,Allocator>& val,
                                            const basic_output_format<Char>& format)
{
    return typename pretty_printer<Char,Allocator>(val,format);
}

inline
char to_hex_character(unsigned char c)
{
    JSONCONS_ASSERT( c <= 0xF );

    return ( c < 10 ) ? ('0' + c) : ('A' - 10 + c);
}

inline
bool is_non_ascii_character(unsigned int c)
{
    return c >= 0x80;
}

template <typename Char>
void escape_string(const std::basic_string<Char>& s,
                   const basic_output_format<Char>& format,
                   std::basic_ostream<Char>& os)
{
    typename std::basic_string<Char>::const_iterator begin = s.begin();
    typename std::basic_string<Char>::const_iterator end = s.end();
    for (typename std::basic_string<Char>::const_iterator it = begin; it != end; ++it)
    {
        Char c = *it;
        switch (c)
        {
        case '\\':
            os << '\\';
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
            unsigned int u(c >= 0 ? c : 256 + c );
            if (is_control_character(u) || (format.escape_all_non_ascii() && is_non_ascii_character(u)))
            {
                // convert utf8 to codepoint
                unsigned int cp = json_char_traits<Char>::char_sequence_to_codepoint(it,end);

                os << '\\';
                os << 'u';
                os << to_hex_character(cp >>12 & 0x000F );
                os << to_hex_character(cp >>8  & 0x000F );
                os << to_hex_character(cp >>4  & 0x000F );
                os << to_hex_character(cp     & 0x000F );
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

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,std::basic_string<Char>>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_string();
    }
    std::basic_string<Char> as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_string();
    }
    void assign(basic_json<Char,Allocator>& self, const std::basic_string<Char>& val)
    {
        self.assign_string(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,basic_json<Char,Allocator>>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return true;
    }
    std::basic_string<Char> as(const basic_json<Char,Allocator>& val) const
    {
        return val;
    }
    void assign(basic_json<Char,Allocator>& self, basic_json<Char,Allocator> val)
    {
        val.swap(self);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,const Char*>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_string();
    }
    const Char* as(const basic_json<Char,Allocator>& val) const
    {
        JSONCONS_ASSERT(val.is_string());
        return val.value_.value_string_.c_str();
    }
    void assign(basic_json<Char,Allocator>& self, const Char* val)
    {
        self.assign_string(std::basic_string<Char>(val));
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,typename basic_json<Char,Allocator>::object>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_object();
    }
    typename basic_json<Char,Allocator>::object as(const basic_json<Char,Allocator>& val) const
    {
        return val;
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,typename basic_json<Char,Allocator>::array>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_array();
    }
    typename basic_json<Char,Allocator>::array as(const basic_json<Char,Allocator>& val) const
    {
        return val;
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,typename basic_json<Char,Allocator>::null_type>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_null();
    }
    typename basic_json<Char,Allocator>::null_type as(const basic_json<Char,Allocator>& val) const
    {
        JSONCONS_ASSERT(val.is_null());
        return basic_json<Char,Allocator>::null_type();
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,bool>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_bool();
    }
    bool as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_bool();
    }
    void assign(basic_json<Char,Allocator>& self, bool val)
    {
        self.assign_bool(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,int>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    int as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_int();
    }
    void assign(basic_json<Char,Allocator>& self, int val)
    {
        self.assign_longlong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,unsigned int>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    unsigned int as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_uint();
    }
    void assign(basic_json<Char,Allocator>& self, unsigned int val)
    {
        self.assign_ulonglong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,short>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    short as(const basic_json<Char,Allocator>& val) const
    {
        return (short)val.as_int();
    }
    void assign(basic_json<Char,Allocator>& self, short val)
    {
        self.assign_longlong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,unsigned short>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    unsigned short as(const basic_json<Char,Allocator>& val) const
    {
        return (unsigned short)val.as_uint();
    }
    void assign(basic_json<Char,Allocator>& self, unsigned short val)
    {
        self.assign_ulonglong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,long>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    long as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_long();
    }
    void assign(basic_json<Char,Allocator>& self, long val)
    {
        self.assign_longlong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,unsigned long>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    unsigned long as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_ulong();
    }
    void assign(basic_json<Char,Allocator>& self, unsigned long val)
    {
        self.assign_ulonglong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,Char>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_string() && val.as_string().size() == 1;
    }
    char as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_char();
    }
    void assign(basic_json<Char,Allocator>& self, Char ch)
    {
        std::basic_string<Char> s = new std::basic_string<Char>();
        s.push_back(ch);
        self.assign_string(s);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,double>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
    {
        return val.is_double();
    }

    double as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_double();
    }
    void assign(basic_json<Char,Allocator>& self, double val)
    {
        self.assign_double(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,long long>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    long long as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_longlong();
    }
    void assign(basic_json<Char,Allocator>& self, long long val)
    {
        self.assign_longlong(val);
    }
};

template <typename Char,typename Allocator>
class value_adapter<Char,Allocator,unsigned long long>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    unsigned long long as(const basic_json<Char,Allocator>& val) const
    {
        return val.as_ulonglong();
    }
    void assign(basic_json<Char,Allocator>& self, unsigned long long val)
    {
        self.assign_ulonglong(val);
    }
};

template <typename Char,typename Allocator,typename T>
class value_adapter<Char,Allocator,std::vector<T>>
{
public:
    bool is(const basic_json<Char,Allocator>& val) const
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
    std::vector<T> as(const basic_json<Char,Allocator>& val) const
    {
        std::vector<T> v(val.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = val[i].template as<T>();
        }
        return v;
    }
};

template <typename Char, typename Allocator>
class basic_json<Char,Allocator>::null_type : public basic_json<Char,Allocator> 
{
private:
    null_type()
        : basic_json<Char,Allocator>(null_t)
    {
    }
};

template <typename Char, typename Allocator>
class basic_json<Char,Allocator>::object : public basic_json<Char,Allocator>
{
public:
    object(json_object<Char,Allocator>* impl)
        : basic_json<Char,Allocator>(impl)
    {
    }
};

template <typename Char, typename Allocator>
class basic_json<Char,Allocator>::array : public basic_json<Char,Allocator>
{
public:
    array(json_array<Char,Allocator>* impl)
        : basic_json<Char,Allocator>(impl)
    {
    }
private:
    array()
        : basic_json<Char,Allocator>(array_t)
    {
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
