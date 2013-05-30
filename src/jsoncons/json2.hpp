// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON2_HPP
#define JSONCONS_JSON2_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include "jsoncons/json1.hpp"
#include "jsoncons/json_variant.hpp"

namespace jsoncons {

// proxy
template <class Char>
template <class Key>
basic_json<Char>::proxy<Key>::proxy(basic_json<Char>& var, Key key)
    : val_(var), key_(key)
{
}

template <class Char>
template <class Key>
size_t basic_json<Char>::proxy<Key>::size() const
{
    return val_.get(key_).size();
}

template <class Char>
template <class Key>
std::string basic_json<Char>::proxy<Key>::as_string() const
{
    return val_.get(key_).as_string();
}

template <class Char>
template <class Key>
bool basic_json<Char>::proxy<Key>::as_bool() const
{
    return val_.get(key_).as_bool();
}

template <class Char>
template <class Key>
double basic_json<Char>::proxy<Key>::as_double() const
{
    return val_.get(key_).as_double();
}

template <class Char>
template <class Key>
int basic_json<Char>::proxy<Key>::as_int() const
{
    return val_.get(key_).as_int();
}

template <class Char>
template <class Key>
unsigned int basic_json<Char>::proxy<Key>::as_uint() const
{
    return val_.get(key_).as_uint();
}

template <class Char>
template <class Key>
long long basic_json<Char>::proxy<Key>::as_longlong() const
{
    return val_.get(key_).as_longlong();
}

template <class Char>
template <class Key>
unsigned long long basic_json<Char>::proxy<Key>::as_ulonglong() const
{
    return val_.get(key_).as_ulonglong();
}

template <class Char>
template <class Key>
basic_json<Char>::proxy<Key>::operator const basic_json<Char>() const
{
    return val_.get(key_);
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(const basic_json& val)
{
    val_.set_member(key_, val);
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<size_t> basic_json<Char>::proxy<Key>::operator[](size_t i)
{
    return proxy<size_t>(val_.get(key_),i);
}

template <class Char>
template <class Key>
const typename basic_json<Char>::proxy<size_t> basic_json<Char>::proxy<Key>::operator[](size_t i) const
{
    return proxy<size_t>(val_.get(key_),i);
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::proxy<Key>::operator[](std::basic_string<Char> name)
{
    return proxy<std::basic_string<Char>>(val_.get(key_),name);
}

template <class Char>
template <class Key>
typename const basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::proxy<Key>::operator[](std::basic_string<Char> name) const
{
    return proxy<std::basic_string<Char>>(val_.get(key_),name);
}

template <class Char>
template <class Key>
std::basic_string<Char> basic_json<Char>::proxy<Key>::to_string() const
{
    return val_.to_string();
}


template <class Char>
basic_json<Char>::basic_json()
{
    type_ = object_t;
    value_.var_ = new json_object<Char>();
}

template <class Char>
template <class Iterator>
basic_json<Char>::basic_json(Iterator begin, Iterator end)
{
    type_ = array_t;
    value_.var_ = new json_array<Char>(begin,end);
}

template <class Char>
basic_json<Char>::basic_json(const basic_json<Char>& val)
{
    type_ = val.type_;
    switch (type_)
    {
    case double_t:
    case longlong_t:
    case ulonglong_t:
    case bool_t:
    case null_t:
        value_ = val.value_;
        break;
    case string_t:
        value_.string_value_.length_ = val.value_.string_value_.length_;
        value_.string_value_.data_ = new Char[value_.string_value_.length_];
        std::memcpy(value_.string_value_.data_,val.value_.string_value_.data_,value_.string_value_.length_*sizeof(Char));
        break;
    default:
        value_.var_ = val.value_.var_->clone();
        break;
    }
}

template <class Char>
basic_json<Char>::basic_json(basic_json&& other)
{
    type_ = other.type_;
    value_ = other.value_;
    other.type_ = null_t;
    other.value_.var_ = nullptr;
}

template <class Char>
basic_json<Char>::basic_json(json_object<Char>* var)
{
    type_ = object_t;
    value_.var_ = var;
}

template <class Char>
basic_json<Char>::basic_json(json_array<Char>* var)
{
    type_ = array_t;
    value_.var_ = var;
}

template <class Char>
basic_json<Char>::basic_json(double val)
{
    type_ = double_t;
    value_.double_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(longlong_type val)
{
    type_ = longlong_t;
    value_.longlong_value_ = val;
}

template <class Char>
basic_json<Char>::basic_json(ulonglong_type val)
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
    value_.string_value_.length_ = s.length();
    value_.string_value_.data_ = new Char[s.length()];
    std::memcpy(value_.string_value_.data_,&s[0],s.length()*sizeof(Char));
}

template <class Char>
basic_json<Char>::basic_json(nullptr_t nullp)
{
    type_ = null_t;
    value_.var_ = nullptr;
}

template <class Char>
basic_json<Char>::~basic_json()
{
    switch (type_)
    {
    case double_t:
    case longlong_t:
    case ulonglong_t:
    case bool_t:
    case null_t:
        break;
    case string_t:
        delete value_.string_value_.data_;
        break;
    default:
        delete value_.var_;
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
void basic_json<Char>::swap(basic_json<Char>& o) throw()
{
    std::swap(type_,o.type_);
    std::swap(value_,o.value_);
}

template <class Char>
basic_json<Char>& basic_json<Char>::get(size_t i) 
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->at(i);
    case array_t:
        return value_.var_->array_cast()->at(i);
    default:
        JSONCONS_THROW_EXCEPTION("Not an array or object");
    }
}

template <class Char>
const basic_json<Char>& basic_json<Char>::get(size_t i) const
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->at(i);
    case array_t:
        return value_.var_->array_cast()->at(i);
    default:
        JSONCONS_THROW_EXCEPTION("Not an array or object");
    }
}

template <class Char>
basic_json<Char>& basic_json<Char>::get(const std::string& name) 
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->get(name);
    default:
        {
            std::ostringstream os;
            os << "Attempting to get " << name << " from a value that is not an object"; 
            JSONCONS_THROW_EXCEPTION(os.str());
        }
    }
}

template <class Char>
const basic_json<Char>& basic_json<Char>::get(const std::string& name) const
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->get(name);
    default:
        {
            std::ostringstream os;
            os << "Attempting to get " << name << " from a value that is not an object"; 
            JSONCONS_THROW_EXCEPTION(os.str());
        }
    }
}

template <class Char>
void basic_json<Char>::set_member(const std::basic_string<Char>& name, const basic_json<Char>& value)
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->set_member(name,value);
    default:
        {
            std::ostringstream os;
            os << "Attempting to set " << name << " on a value that is not an object"; 
            JSONCONS_THROW_EXCEPTION(os.str());
        }
    }
}

template <class Char>
void basic_json<Char>::set_member(std::basic_string<Char>&& name, basic_json<Char>&& value)
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->set_member(name,value);
    default:
        {
            std::ostringstream os;
            os << "Attempting to set " << name << " on a value that is not an object"; 
            JSONCONS_THROW_EXCEPTION(os.str());
        }
    }
}

template <class Char>
size_t basic_json<Char>::size() const
{
    switch (type_)
    {
    case object_t:
        return value_.var_->object_cast()->size();
    case array_t:
        return value_.var_->array_cast()->size();
    default:
        return 0;
    }
}

template <class Char>
typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::operator[](std::basic_string<Char> name)
{
    return proxy<std::basic_string<Char>>(*this,name);
}

template <class Char>
const typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::operator[](std::basic_string<Char> name) const
{
    return proxy<std::basic_string<Char>>(*this,name);
}

template <class Char>
typename basic_json<Char>::proxy<size_t> basic_json<Char>::operator[](size_t i)
{
    return proxy<size_t>(*this,name);
}

template <class Char>
const typename basic_json<Char>::proxy<size_t> basic_json<Char>::operator[](size_t i) const
{
    return proxy<size_t>(*this,name);
}

template <class Char>
std::basic_string<Char> basic_json<Char>::to_string() const
{
    std::basic_ostringstream<Char> os;
    os.precision(16);
    to_stream(os);
    return os.str();
}

template <class Char>
void basic_json<Char>::to_stream(std::basic_ostream<Char>& os) const
{
    switch (type_)
    {
    case string_t:
        os << "\"" << value_.string_value_ << "\"";
        break;
    case double_t:
        os << value_.double_value_;
        break;
    case longlong_t:
        os << value_.longlong_value_;
        break;
    case ulonglong_t:
        os << value_.ulonglong_value_;
        break;
    case bool_t:
        os << (value_.bool_value_ ? "true" : "false");
        break;
    case null_t:
        os << "null";
        break;
    default:
        value_.var_->to_stream(os);
        break;
    }
}

template <class Char>
const basic_json<Char> basic_json<Char>::object_prototype(new json_object<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::array_prototype(new json_array<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::null_prototype(basic_json<Char>(nullptr));

template <class Char> 
basic_json<Char> basic_json<Char>::parse(std::basic_istream<Char>& is)
{
    json_parser<Char> parser;
    json_content_handler<Char> handler;
    parser.parse(is,handler);
    basic_json<Char> val(nullptr);
    val.swap(handler.root_);
    return val;
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    json_parser<Char> parser;
    json_content_handler<Char> handler;
    parser.parse(is,handler);
    basic_json<Char> val(nullptr);
    val.swap(handler.root_);
    return val;
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
    default:
        JSONCONS_THROW_EXCEPTION("Not a double");
    }
}

template <class Char>
bool basic_json<Char>::is_empty() const
{
    switch (type_)
    {
    case null_t:
        return true;
    case array_t:
        return value_.array_cast()->size() == 0;
    case object_t:
        return value_.object_cast()->size() == 0;
        return true;
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
std::string basic_json<Char>::as_string() const
{
    switch (type_)
    {
    case string_t:
        return std::string(value_.string_value_.data_,value_.string_value_.length_);
    default:
        return to_string();
    }
}

template <class Char>
std::ostream& operator<<(std::ostream& os, const basic_json<Char>& o)
{
    os << o.to_string();
    return os;
}

template <class Char,class Key>
std::ostream& operator<<(std::ostream& os, typename const basic_json<Char>::proxy<Key>& o)
{
    os << o.to_string();
    return os;
}

template <class Char>
std::basic_string<Char> escape_string(const std::basic_string<Char>& s)
{
   	std::basic_string<Char> buf;
       for (size_t i = 0; i < s.length(); ++i)
       {
           Char c = s[i];
           switch (c)
           {
           case '\\':
               buf.push_back('\\');
               break;
           case '"':
               buf.push_back('\\');
               buf.push_back('\"');
               break;
           case '\b':
               buf.push_back('\\');
               buf.push_back('b');
               break;
           case '\f':
               buf.push_back('\\');
               buf.push_back('f');
               break;
           case '\n':
               buf.push_back('\\');
               buf.push_back('n');
               break;
           case '\r':
               buf.push_back('\\');
               buf.push_back('r');
               break;
           case '\t':
               buf.push_back('\\');
               buf.push_back('t');
               break;
           default:
               buf.push_back(c);
               break;
           }
       }
       return buf;
}

}

#endif
