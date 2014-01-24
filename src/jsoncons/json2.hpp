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


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"


namespace jsoncons {

template <class Char>
basic_json<Char>::basic_json()
{
    type_ = empty_object_t;
}

template <class Char>
template <class InputIterator>
basic_json<Char>::basic_json(InputIterator first, InputIterator last)
{
    type_ = array_t;
    value_.array_ = new json_array<Char>(first,last);
}

template <class Char>
basic_json<Char>::basic_json(const basic_json<Char>& val)
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
        value_.string_value_ = new std::basic_string<Char>(*(val.value_.string_value_));
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

template <class Char>
basic_json<Char>::basic_json(json_object<Char>* var)
{
    type_ = object_t;
    value_.object_ = var;
}

template <class Char>
basic_json<Char>::basic_json(json_array<Char>* var)
{
    type_ = array_t;
    value_.array_ = var;
}

template <class Char>
basic_json<Char>::basic_json(basic_custom_data<Char>* var)
{
    type_ = custom_t;
    value_.userdata_ = var;
}

template <class Char>
basic_json<Char>::basic_json(double val)
{
    type_ = double_t;
    value_.double_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(long long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(int val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(unsigned int val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(long val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(unsigned long val)
{
    type_ = ulonglong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(unsigned long long val)
{
    type_ = ulonglong_t;
    value_.ulonglong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(bool val)
{
    type_ = bool_t;
    value_.bool_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(const std::basic_string<Char>& s)
{
    type_ = string_t;
    value_.string_value_ = new std::basic_string<Char>(s);
}

template <class Char>
basic_json<Char>::basic_json(const Char* s)
{
    type_ = string_t;
    value_.string_value_ = new std::basic_string<Char>(s);
}

template <class Char>
basic_json<Char>::basic_json(value_type t)
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
        value_.string_value_ = new std::basic_string<Char>();
        break;
    case array_t:
        value_.array_ = new json_array<Char>();
        break;
    case object_t:
        value_.object_ = new json_object<Char>();
        break;

    case custom_t:
        assert(false);
    }
}

template <class Char>
basic_json<Char>::~basic_json()
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

template <class Char>
basic_json<Char>& basic_json<Char>::operator=(basic_json<Char> rhs)
{
    rhs.swap(*this);
    return *this;
}

template <class Char>
bool basic_json<Char>::operator!=(const basic_json<Char>& rhs) const
{
    return !(*this == rhs);
}

template <class Char>
bool basic_json<Char>::operator==(const basic_json<Char>& rhs) const
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

template <class Char>
basic_json<Char>& basic_json<Char>::at(size_t i) 
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

template <class Char>
const basic_json<Char>& basic_json<Char>::at(size_t i) const
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

template <class Char>
basic_json<Char>& basic_json<Char>::at(const std::basic_string<Char>& name) 
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

template <class Char>
const basic_json<Char>& basic_json<Char>::at(const std::basic_string<Char>& name) const
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

template <class Char>
const basic_json<Char>& basic_json<Char>::get(const std::basic_string<Char>& name) const
{
    switch (type_)
    {
    case empty_object_t:
        return basic_json<Char>::null;
    case object_t:
		{
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? it->second : basic_json<Char>::null;
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <class Char>
typename basic_json<Char>::const_val_proxy basic_json<Char>::get(const std::basic_string<Char>& name, const basic_json<Char>& default_val) const
{
    switch (type_)
    {
    case empty_object_t:
        return const_val_proxy(default_val);
    case object_t:
		{
        const_object_iterator it = value_.object_->find(name);
        return it != end_members() ? const_val_proxy(it->second) : const_val_proxy(default_val);
		}
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to get %s from a value that is not an object", name);
        }
    }
}

template <class Char>
void basic_json<Char>::set(const std::basic_string<Char>& name, const basic_json<Char>& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
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

template <class Char>
basic_json<Char>::basic_json(basic_json&& other)
{
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = null_t;
}

template <class Char>
void basic_json<Char>::add(basic_json<Char>&& value)
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

template <class Char>
void basic_json<Char>::add(size_t index, basic_json<Char>&& value)
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

template <class Char>
void basic_json<Char>::set(std::basic_string<Char>&& name, basic_json<Char>&& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
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

template <class Char>
template <class T>
void basic_json<Char>::set_custom_data(const std::basic_string<Char>& name, const T& value)
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
    case object_t:
        value_.object_->set(name,basic_json<Char>(new custom_data_wrapper<Char,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
        }
    }
}

template <class Char>
void basic_json<Char>::clear()
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

template <class Char>
void basic_json<Char>::remove_range(size_t from_index, size_t to_index)
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

template <class Char>
void basic_json<Char>::remove_member(const std::basic_string<Char>& name)
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

template <class Char>
void basic_json<Char>::add(const basic_json<Char>& value)
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

template <class Char>
void basic_json<Char>::add(size_t index, const basic_json<Char>& value)
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

template <class Char>
template <class T>
void basic_json<Char>::add_custom_data(const T& value)
{
    switch (type_)
    {
    case array_t:
		{
        add(basic_json<Char>(new custom_data_wrapper<Char,T>(value)));
		}
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <class Char>
template <class T>
void basic_json<Char>::add_custom_data(size_t index, const T& value)
{
    switch (type_)
    {
    case array_t:
        value_.array_->add(index,basic_json<Char>(new custom_data_wrapper<Char,T>(value)));
        break;
    default:
        {
            JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
        }
    }
}

template <class Char>
size_t basic_json<Char>::size() const
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

template <class Char>
typename basic_json<Char>::object_key_proxy basic_json<Char>::operator[](const std::basic_string<Char>& name)
{
    return object_key_proxy(*this,name);
}

template <class Char>
const basic_json<Char>& basic_json<Char>::operator[](const std::basic_string<Char>& name) const
{
    return at(name);
}

template <class Char>
basic_json<Char>& basic_json<Char>::operator[](size_t i)
{
    return value_.array_->at(i);
}

template <class Char>
const basic_json<Char>& basic_json<Char>::operator[](size_t i) const
{
    return value_.array_->at(i);
}

template <class Char>
std::basic_string<Char> basic_json<Char>::to_string() const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer);
    return os.str();
}

template <class Char>
std::basic_string<Char> basic_json<Char>::to_string(const basic_output_format<Char>& format) const
{
    std::basic_ostringstream<Char> os;
    basic_json_serializer<Char> serializer(os,format);
    to_stream(serializer);
    return os.str();
}

template <class Char>
void basic_json<Char>::to_stream(basic_json_output_handler<Char>& handler) const
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
        json_object<Char>* o = value_.object_;
        for (const_object_iterator it = o->begin(); it != o->end(); ++it)
        {
            handler.name(it->first);
            it->second.to_stream(handler);
        }
        handler.end_object();
		}
        break;
    case array_t:
		{
        handler.begin_array();
        json_array<Char>* o = value_.array_;
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

template <class Char>
void basic_json<Char>::to_stream(std::basic_ostream<Char>& os) const
{
    basic_json_serializer<Char> serializer(os);
    to_stream(serializer); 
}

template <class Char>
void basic_json<Char>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
{
    basic_json_serializer<Char> serializer(os,format);
    to_stream(serializer);
}

template <class Char>
void basic_json<Char>::to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const
{
    basic_json_serializer<Char> serializer(os,format,indenting);
    to_stream(serializer);
}

template <class Char>
const basic_json<Char> basic_json<Char>::an_object(new json_object<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::an_array(new json_array<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::null = basic_json<Char>(basic_json<Char>::null_t);

template <class Char> 
basic_json<Char> basic_json<Char>::make_array()
{
    return basic_json<Char>(new json_array<Char>());
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_array(size_t n)
{
    return basic_json<Char>(new json_array<Char>(n));
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_array(size_t n, const basic_json<Char>& val)
{
    return basic_json<Char>(new json_array<Char>(n,val));
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_2d_array(size_t m, size_t n)
{
    basic_json<Char> a(basic_json<Char>(new json_array<Char>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char>::make_array(n);
    }
    return a;
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_2d_array(size_t m, size_t n, const basic_json<Char>& val)
{
    basic_json<Char> a(basic_json<Char>(new json_array<Char>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char>::make_array(n,val);
    }
    return a;
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<Char> a(basic_json<Char>(new json_array<Char>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char>::make_2d_array(n,k);
    }
    return a;
}

template <class Char> 
basic_json<Char> basic_json<Char>::make_3d_array(size_t m, size_t n, size_t k, const basic_json<Char>& val)
{
    basic_json<Char> a(basic_json<Char>(new json_array<Char>(m)));
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<Char>::make_2d_array(n,k,val);
    }
    return a;
}

template <class Char> 
basic_json<Char> basic_json<Char>::parse(std::basic_istream<Char>& is)
{
    basic_json_deserializer<Char> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char> val;
    handler.root().swap(val);
    return val;
}

template <class Char>
basic_json<Char> basic_json<Char>::parse_string(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    basic_json_deserializer<Char> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char> val;
    handler.root().swap(val);
    return val;
}

template <class Char>
basic_json<Char> basic_json<Char>::parse_file(const std::string& filename)
{
    std::basic_ifstream<Char> is(filename.c_str(), std::basic_ifstream<Char>::in | std::basic_ifstream<Char>::binary);
    if (!is.is_open())
    {
        throw json_exception_1<char>("Cannot open file %s", filename);
    }
 
    basic_json_deserializer<Char> handler;
    basic_json_reader<Char> parser(is,handler);
    parser.read();
    basic_json<Char> val;
    handler.root().swap(val);
    return val;
}

template <class Char>
typename basic_json<Char>::object_iterator basic_json<Char>::begin_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
    case object_t:
        return value_.object_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <class Char>
typename basic_json<Char>::const_object_iterator basic_json<Char>::begin_members() const
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

template <class Char>
typename basic_json<Char>::object_iterator basic_json<Char>::end_members()
{
    switch (type_)
    {
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
    case object_t:
        return value_.object_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an object");
    }
}

template <class Char>
typename basic_json<Char>::const_object_iterator basic_json<Char>::end_members() const
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

template <class Char>
typename basic_json<Char>::array_iterator basic_json<Char>::begin_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class Char>
typename basic_json<Char>::const_array_iterator basic_json<Char>::begin_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->begin();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class Char>
typename basic_json<Char>::array_iterator basic_json<Char>::end_elements()
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class Char>
typename basic_json<Char>::const_array_iterator basic_json<Char>::end_elements() const
{
    switch (type_)
    {
    case array_t:
        return value_.array_->end();
    default:
        JSONCONS_THROW_EXCEPTION("Not an array");
    }
}

template <class Char>
double basic_json<Char>::as_double() const
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

template <class Char>
bool basic_json<Char>::is_empty() const
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

template <class Char>
void basic_json<Char>::reserve(size_t n) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->reserve(n);
        break;
    case empty_object_t:
        type_ = object_t;
        value_.object_ = new json_object<Char>();
    case object_t:
        value_.object_->reserve(n);
        break;
    }
}

template <class Char>
void basic_json<Char>::resize_array(size_t n) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n);
        break;
    }
}

template <class Char>
void basic_json<Char>::resize_array(size_t n, const basic_json<Char>& val) 
{
    switch (type_)
    {
    case array_t:
        value_.array_->resize(n,val);
        break;
    }
}

template <class Char>
size_t basic_json<Char>::capacity() const
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

template <class Char>
bool basic_json<Char>::has_member(const std::basic_string<Char>& name) const
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

template <class Char>
bool basic_json<Char>::as_bool() const
{
    switch (type_)
    {
    case bool_t:
        return value_.bool_value_;
    default:
        JSONCONS_THROW_EXCEPTION("Not a int");
    }
}

template <class Char>
int basic_json<Char>::as_int() const
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

template <class Char>
unsigned int basic_json<Char>::as_uint() const
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

template <class Char>
long long basic_json<Char>::as_longlong() const
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

template <class Char>
unsigned long long basic_json<Char>::as_ulonglong() const
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

template <class Char>
long basic_json<Char>::as_long() const
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

template <class Char>
unsigned long basic_json<Char>::as_ulong() const
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

template <class Char>
template <class T>
const T& basic_json<Char>::custom_data() const
{
    switch (type_)
    {
    case custom_t:
        return static_cast<const custom_data_wrapper<Char,T>*>(value_.userdata_)->data1_;
    default:
        JSONCONS_THROW_EXCEPTION("Not userdata");
    }
}

template <class Char>
template <class T>
T& basic_json<Char>::custom_data() 
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

template <class Char>
std::basic_string<Char> basic_json<Char>::as_string() const
{
    switch (type_)
    {
    case string_t:
        return *(value_.string_value_);
    default:
        return to_string();
    }
}

template <class Char>
std::basic_string<Char> basic_json<Char>::as_string(const basic_output_format<Char>& format) const
{
    switch (type_)
    {
    case string_t:
        return *(value_.string_value_);
    default:
        return to_string(format);
    }
}

template <class Char>
Char basic_json<Char>::as_char() const
{
    switch (type_)
    {
    case string_t:
        return value_.string_value_->length() > 0 ? (*value_.string_value_)[0] : '\0';
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

template <class Char>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const basic_json<Char>& o)
{
    o.to_stream(os);
    return os;
}

template <class Char>
class basic_pretty_print
{
public:

    basic_pretty_print(basic_json<Char>& o)
        : o_(o)
    {
    }

    basic_pretty_print(basic_json<Char>& o,
                       const basic_output_format<Char>& format)
        : o_(o), format_(format)
    {
	;
    }

    void to_stream(std::basic_ostream<Char>& os) const
    {
        o_.to_stream(os,format_,true);
    }

    const basic_json<Char>& o_;
    basic_output_format<Char> format_;
private:
    basic_pretty_print();
    basic_pretty_print(const basic_pretty_print& o);
};

template <class Char>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const basic_pretty_print<Char>& o)
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

template <class Char>
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

}

#pragma GCC diagnostic pop
#endif
