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
    return val_.get(key_).to_string();
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
    value_.var_ = val.value_.var_->clone();

}

template <class Char>
basic_json<Char>::basic_json(basic_json&& other)
{
    type_ = other.type_;
    value_.var_ = other.value_.var_;
    other.value_.var_ = nullptr;
}

template <class Char>
basic_json<Char>::basic_json(json_variant<Char>* var)
{
    value_.var_ = var;
}

template <class Char>
basic_json<Char>::basic_json(double val)
{
    type_ = double_t;
    value_.var_ = new json_double<Char>(val);
}

template <class Char>
basic_json<Char>::basic_json(longlong_type val)
{
    type_ = longlong_t;
    value_.var_ = new json_integer<Char>(val);
}

template <class Char>
basic_json<Char>::basic_json(ulonglong_type val)
{
    type_ = ulonglong_t;
    value_.var_ = new json_uinteger<Char>(val);
}

template <class Char>
basic_json<Char>::basic_json(bool val)
{
    type_ = bool_t;
    value_.var_ = new json_bool<Char>(val);
}

template <class Char>
basic_json<Char>::basic_json(std::string s)
{
    type_ = string_t;
    value_.var_ = new json_string<Char>(s);
}

template <class Char>
basic_json<Char>::basic_json(nullptr_t nullp)
{
    type_ = null_t;
    value_.var_ = new json_null<Char>();
}

template <class Char>
basic_json<Char>::~basic_json()
{
    delete value_.var_;
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
    std::swap(value_,o.value_);
}

template <class Char>
basic_json<Char>& basic_json<Char>::get(size_t i) 
{
    return value_.var_->get(i);
}

template <class Char>
const basic_json<Char>& basic_json<Char>::get(size_t i) const
{
    return value_.var_->get(i);
}

template <class Char>
basic_json<Char>& basic_json<Char>::get(const std::string& name) 
{
    return value_.var_->get(name);
}

template <class Char>
const basic_json<Char>& basic_json<Char>::get(const std::string& name) const
{
    return value_.var_->get(name);
}

template <class Char>
void basic_json<Char>::set_member(const std::basic_string<Char>& name, basic_json<Char> value)
{
    return value_.var_->set_member(name,value);
}

template <class Char>
size_t basic_json<Char>::size() const
{
    return value_.var_->size();
}

template <class Char>
bool basic_json<Char>::is_null() const
{
    return value_.var_->is_null();
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
    return value_.var_->to_string();
}

template <class Char>
void basic_json<Char>::to_stream(std::ostream& os) const
{
    value_.var_->to_stream(os);
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
    json_variant<Char>* var = 0;
    std::swap(var,handler.root_.value_.var_);
    return basic_json<Char>(var);
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    json_parser<Char> parser;
    json_content_handler<Char> handler;
    json_variant<Char>* var = parser.parse(is,handler);
    json_variant<Char>* var = 0;
    std::swap(var,handler.root_.value_.var_);
    return basic_json<Char>(var);
}

template <class Char>
double basic_json<Char>::as_double() const
{
    return value_.var_->as_double();
}

template <class Char>
int basic_json<Char>::as_int() const
{
    return value_.var_->as_int();
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

}

#endif
