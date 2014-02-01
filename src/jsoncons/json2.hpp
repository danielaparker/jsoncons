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

namespace jsoncons {

template <class C>
basic_json<C>::basic_json()
{
    type_ = empty_object_t;
}

template <class C>
template <class InputIterator>
basic_json<C>::basic_json(InputIterator first, InputIterator last)
{
    type_ = array_t;
    value_.array_ = new json_array<C>(first,last);
}

template <class C>
basic_json<C>::basic_json(const basic_json<C>& val)
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
        value_.string_value_ = new std::basic_string<C>(*(val.value_.string_value_));
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

template <class C>
basic_json<C>::basic_json(json_object<C>* var)
{
    type_ = object_t;
    value_.object_ = var;
}

template <class C>
basic_json<C>::basic_json(json_array<C>* var)
{
    type_ = array_t;
    value_.array_ = var;
}

template <class C>
basic_json<C>::basic_json(basic_custom_data<C>* var)
{
    type_ = custom_t;
    value_.userdata_ = var;
}

template <class C>
basic_json<C>::basic_json(double val)
{
    type_ = double_t;
    value_.double_value_ = val;
}

template <class C>
basic_json<C>::basic_json(long long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(int val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(unsigned int val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(unsigned long val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(unsigned long long val)
{
    type_ = ulonglong_t;
    value_.ulonglong_value_ = val;
}

template <class C>
basic_json<C>::basic_json(bool val)
{
    type_ = bool_t;
    value_.bool_value_ = val;
}

template <class C>
basic_json<C>::basic_json(const std::basic_string<C>& s)
{
    type_ = string_t;
    value_.string_value_ = new std::basic_string<C>(s);
}

template <class C>
basic_json<C>::basic_json(const C* s)
{
    type_ = string_t;
    value_.string_value_ = new std::basic_string<C>(s);
}

template <class C>
basic_json<C>::basic_json(value_type t)
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
        value_.string_value_ = new std::basic_string<C>();
        break;
    case array_t:
        value_.array_ = new json_array<C>();
        break;
    case object_t:
        value_.object_ = new json_object<C>();
        break;
    }
}

template <class C>
basic_json<C>::~basic_json()
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
        delete value_.string_value_;
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

template <class C>
basic_json<C>& basic_json<C>::operator=(const char* rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = string_t;
        value_.string_value_ = new std::basic_string<C>(rhs);
        break;
    default:
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(const std::basic_string<C>& rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = string_t;
        value_.string_value_ = new std::basic_string<C>(rhs);
        break;
    default:
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(int rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(unsigned int rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(long rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(unsigned long rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(long long rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(unsigned long long rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(bool rhs)
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
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(double rhs)
{
    switch (type_)
    {
    case null_t:
    case bool_t:
    case empty_object_t:
	case longlong_t:
	case double_t:
        type_ = double_t;
        value_.double_value_ = rhs;
        break;
    default:
        basic_json<C>(rhs).swap(*this);
        break;
    }
    return *this;
}

template <class C>
basic_json<C>& basic_json<C>::operator=(basic_json<C> rhs)
{
    rhs.swap(*this);
    return *this;
}

template <class C>
bool basic_json<C>::operator!=(const basic_json<C>& rhs) const
{
    return !(*this == rhs);
}

template <class C>
bool basic_json<C>::operator==(const basic_json<C>& rhs) const
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
        return *(value_.string_value_) == *(rhs.value_.string_value_);
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

template <class C>
basic_json<C>& basic_json<C>::at(size_t i) 
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

template <class C>
const basic_json<C>& basic_json<C>::at(size_t i) const
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

template <class C>
basic_json<C>& basic_json<C>::at(const std::basic_string<C>& name) 
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

template <class C>
const basic_json<C>& basic_json<C>::at(const std::basic_string<C>& name) const
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

template <class C>
const basic_json<C>& basic_json<C>::get(const std::basic_string<C>& name) const
{
    switch (type_)
    {
    case empty_object_t:
        return basic_json<C>::null;
    case object_t:
		{
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? it->value : basic_json<C>::null;
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <class C>
typename basic_json<C>::const_val_proxy basic_json<C>::get(const std::basic_string<C>& name, const basic_json<C>& default_val) const
{
    switch (type_)
    {
    case empty_object_t:
        return const_val_proxy(default_val);
    case object_t:
		{
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? const_val_proxy(it->value) : const_val_proxy(default_val);
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <class C>
void basic_json<C>::set(const std::basic_string<C>& name, const basic_json<C>& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        value_.object_->set(name,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

#ifndef JSONCONS_NO_CXX11_RVALUE_REFERENCES

template <class C>
basic_json<C>::basic_json(basic_json&& other)
{
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = null_t;
}

template <class C>
void basic_json<C>::add(basic_json<C>&& value)
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

template <class C>
void basic_json<C>::add(size_t index, basic_json<C>&& value)
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

template <class C>
void basic_json<C>::set(std::basic_string<C>&& name, basic_json<C>&& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        value_.object_->set(name,value);
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

#endif

template <class C>
template <class T>
void basic_json<C>::set_custom_data(const std::basic_string<C>& name, const T& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        value_.object_->set(name,basic_json<C>(new custom_data_wrapper<C,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

template <class C>
void basic_json<C>::clear()
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

template <class C>
void basic_json<C>::remove_range(size_t from_index, size_t to_index)
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

template <class C>
void basic_json<C>::remove_member(const std::basic_string<C>& name)
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

template <class C>
void basic_json<C>::add(const basic_json<C>& value)
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

template <class C>
void basic_json<C>::add(size_t index, const basic_json<C>& value)
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

template <class C>
template <class T>
void basic_json<C>::add_custom_data(const T& value)
{
    switch (type_)
    {
    case array_t:
		{
        add(basic_json<C>(new custom_data_wrapper<C,T>(value)));
		}
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <class C>
template <class T>
void basic_json<C>::add_custom_data(size_t index, const T& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index,basic_json<C>(new custom_data_wrapper<C,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <class C>
size_t basic_json<C>::size() const
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

template <class C>
typename basic_json<C>::object_key_proxy basic_json<C>::operator[](const std::basic_string<C>& name)
{
    return object_key_proxy(*this,name);
}

template <class C>
const basic_json<C>& basic_json<C>::operator[](const std::basic_string<C>& name) const
{
    return at(name);
}

template <class C>
basic_json<C>& basic_json<C>::operator[](size_t i)
{
    return value_.array_->at(i);
}

template <class C>
const basic_json<C>& basic_json<C>::operator[](size_t i) const
{
    return value_.array_->at(i);
}

template <class C>
std::basic_string<C> basic_json<C>::to_string() const
{
    std::basic_ostringstream<C> os;
    basic_json_serializer<C> serializer(os);
    to_stream(serializer);
    return os.str();
}

template <class C>
std::basic_string<C> basic_json<C>::to_string(const basic_output_format<C>& format) const
{
    std::basic_ostringstream<C> os;
    basic_json_serializer<C> serializer(os,format);
    to_stream(serializer);
    return os.str();
}

template <class C>
void basic_json<C>::to_stream(basic_json_output_handler<C>& handler) const
{
    switch (type_)
    {
    case string_t:
        handler.value(*(value_.string_value_));
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
        handler.null_value();
        break;
    case empty_object_t:
        handler.begin_object();
        handler.end_object();
        break;
    case object_t:
		{
        handler.begin_object();
        json_object<C>* o = value_.object_;
        for (const_object_iterator it = o->begin(); it != o->end(); ++it)
        {
            handler.name(it->name);
            it->value.to_stream(handler);
        }
        handler.end_object();
		}
        break;
    case array_t:
		{
        handler.begin_array();
        json_array<C>* o = value_.array_;
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

template <class C>
void basic_json<C>::to_stream(std::basic_ostream<C>& os) const
{
    basic_json_serializer<C> serializer(os);
    to_stream(serializer); 
}

template <class C>
void basic_json<C>::to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format) const
{
    basic_json_serializer<C> serializer(os,format);
    to_stream(serializer);
}

template <class C>
void basic_json<C>::to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format, bool indenting) const
{
    basic_json_serializer<C> serializer(os,format,indenting);
    to_stream(serializer);
}

template <class C>
const basic_json<C> basic_json<C>::an_object(new json_object<C>());

template <class C>
const basic_json<C> basic_json<C>::an_array(new json_array<C>());

template <class C>
const basic_json<C> basic_json<C>::null = basic_json<C>(basic_json<C>::null_t);

template <class C> 
basic_json<C> basic_json<C>::make_array()
{
    return basic_json<C>(new json_array<C>());
}

template <class C> 
basic_json<C> basic_json<C>::make_array(size_t n)
{
    return basic_json<C>(new json_array<C>(n));
}

template <class C> 
basic_json<C> basic_json<C>::make_array(size_t n, const basic_json<C>& val)
{
    return basic_json<C>(new json_array<C>(n,val));
}

template <class C> 
basic_json<C> basic_json<C>::make_2d_array(size_t m, size_t n)
{
    basic_json<C> a(basic_json<C>(new json_array<C>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<C>::make_array(n);
    }
    return a;
}

template <class C> 
basic_json<C> basic_json<C>::make_2d_array(size_t m, size_t n, const basic_json<C>& val)
{
    basic_json<C> a(basic_json<C>(new json_array<C>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<C>::make_array(n,val);
    }
    return a;
}

template <class C> 
basic_json<C> basic_json<C>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<C> a(basic_json<C>(new json_array<C>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<C>::make_2d_array(n,k);
    }
    return a;
}

template <class C> 
basic_json<C> basic_json<C>::make_3d_array(size_t m, size_t n, size_t k, const basic_json<C>& val)
{
    basic_json<C> a(basic_json<C>(new json_array<C>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<C>::make_2d_array(n,k,val);
    }
    return a;
}

template <class C> 
basic_json<C> basic_json<C>::parse(std::basic_istream<C>& is)
{
    basic_json_deserializer<C> handler;
    basic_json_reader<C> parser(is,handler);
    parser.read();
    basic_json<C> val;
    handler.root().swap(val);
    return val;
}

template <class C>
basic_json<C> basic_json<C>::parse_string(const std::basic_string<C>& s)
{
    std::basic_istringstream<C> is(s);
    basic_json_deserializer<C> handler;
    basic_json_reader<C> parser(is,handler);
    parser.read();
    basic_json<C> val;
    handler.root().swap(val);
    return val;
}

template <class C>
basic_json<C> basic_json<C>::parse_file(const std::string& filename)
{
    std::basic_ifstream<C> is(filename.c_str(), std::basic_ifstream<C>::in | std::basic_ifstream<C>::binary);
    if (!is.is_open())
    {
        throw json_exception_1<char>("Cannot open file %s", filename);
    }
 
    basic_json_deserializer<C> handler;
    basic_json_reader<C> parser(is,handler);
    parser.read();
    basic_json<C> val;
    handler.root().swap(val);
    return val;
}

template <class C>
typename basic_json<C>::object_iterator basic_json<C>::begin_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        return value_.object_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <class C>
typename basic_json<C>::const_object_iterator basic_json<C>::begin_members() const
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

template <class C>
typename basic_json<C>::object_iterator basic_json<C>::end_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        return value_.object_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <class C>
typename basic_json<C>::const_object_iterator basic_json<C>::end_members() const
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

template <class C>
typename basic_json<C>::array_iterator basic_json<C>::begin_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class C>
typename basic_json<C>::const_array_iterator basic_json<C>::begin_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class C>
typename basic_json<C>::array_iterator basic_json<C>::end_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class C>
typename basic_json<C>::const_array_iterator basic_json<C>::end_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class C>
double basic_json<C>::as_double() const
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

template <class C>
bool basic_json<C>::is_empty() const
{
    switch (type_)
    {
    case string_t:
        return value_.string_value_->size() == 0;
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

template <class C>
void basic_json<C>::reserve(size_t n) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->reserve(n);
        break;
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<C>();
    case object_t:
        value_.object_->reserve(n);
        break;
    }
}

template <class C>
void basic_json<C>::resize_array(size_t n) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n);
        break;
    }
}

template <class C>
void basic_json<C>::resize_array(size_t n, const basic_json<C>& val) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n,val);
        break;
    }
}

template <class C>
size_t basic_json<C>::capacity() const
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

template <class C>
bool basic_json<C>::has_member(const std::basic_string<C>& name) const
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

template <class C>
bool basic_json<C>::as_bool() const
{
    switch (type_)
    {
    case bool_t:
        return value_.bool_value_;
    default:
        JSONCONS_THROW_EXCEPTION("Not a int");
    }
}

template <class C>
int basic_json<C>::as_int() const
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

template <class C>
unsigned int basic_json<C>::as_uint() const
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

template <class C>
long long basic_json<C>::as_longlong() const
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

template <class C>
unsigned long long basic_json<C>::as_ulonglong() const
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

template <class C>
long basic_json<C>::as_long() const
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

template <class C>
unsigned long basic_json<C>::as_ulong() const
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

template <class C>
template <class T>
const T& basic_json<C>::custom_data() const
{
    switch (type_)
    {
    case custom_t:
        return static_cast<const custom_data_wrapper<C,T>*>(value_.userdata_)->data1_;
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template <class C>
template <class T>
T& basic_json<C>::custom_data() 
{
    switch (type_)
    {
    case custom_t:
		{
        custom_data_wrapper<C,T>* p = static_cast<custom_data_wrapper<C,T>*>(value_.userdata_);
        return p->data1_;
		}
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template <class C>
std::basic_string<C> basic_json<C>::as_string() const
{
    switch (type_)
    {
    case string_t:
        return *(value_.string_value_);
    default:
        return to_string();
    }
}

template <class C>
std::basic_string<C> basic_json<C>::as_string(const basic_output_format<C>& format) const
{
    switch (type_)
    {
    case string_t:
        return *(value_.string_value_);
    default:
        return to_string(format);
    }
}

template <class C>
C basic_json<C>::as_char() const
{
    switch (type_)
    {
    case string_t:
        return value_.string_value_->length() > 0 ? (*value_.string_value_)[0] : '\0';
    case longlong_t:
        return static_cast<C>(value_.longlong_value_);
    case ulonglong_t:
        return static_cast<C>(value_.ulonglong_value_);
    case double_t:
        return static_cast<C>(value_.double_value_);
    case bool_t:
        return value_.bool_value_ ? 1 : 0;
    case null_t:
        return 0;
    default:
        JSONCONS_THROW_EXCEPTION("Cannot convert to char");
    }
}

template <class C>
std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, const basic_json<C>& o)
{
    o.to_stream(os);
    return os;
}

template <class C>
class basic_pretty_print
{
public:

    basic_pretty_print(basic_json<C>& o)
        : o_(o)
    {
    }

    basic_pretty_print(basic_json<C>& o,
                       const basic_output_format<C>& format)
        : o_(o), format_(format)
    {
	;
    }

    void to_stream(std::basic_ostream<C>& os) const
    {
        o_.to_stream(os,format_,true);
    }

    const basic_json<C>& o_;
    basic_output_format<C> format_;
private:
    basic_pretty_print();
    basic_pretty_print(const basic_pretty_print& o);
};

template <class C>
std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, const basic_pretty_print<C>& o)
{
    o.to_stream(os);
    return os;
}

typedef basic_pretty_print<char> pretty_print;

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

template <class C>
void escape_string(const std::basic_string<C>& s, 
                   const basic_output_format<C>& format,
                   std::basic_ostream<C>& os)
{
    typename std::basic_string<C>::const_iterator begin = s.begin();
    typename std::basic_string<C>::const_iterator end = s.end();
    for (typename std::basic_string<C>::const_iterator it = begin; it != end; ++it)
    {
        C c = *it;
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
                unsigned int cp = json_char_traits<C>::char_sequence_to_codepoint(it,end);

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

}

#endif
