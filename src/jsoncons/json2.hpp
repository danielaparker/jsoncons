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
basic_json<Char>::proxy<Key>::proxy(json_variant<Char>* var, Key key)
    : var_(var), key_(key)
{
}

template <class Char>
template <class Key>
size_t basic_json<Char>::proxy<Key>::size() const
{
    return var_->get(key_).size();
}

template <class Char>
template <class Key>
basic_json<Char>::proxy<Key>::operator const basic_json<Char>() const
{
    return var_->get(key_);
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(const basic_json& val)
{
    var_->set_member(key_, val.var_->clone());
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<size_t> basic_json<Char>::proxy<Key>::operator[](size_t i)
{
    return proxy<size_t>(var_->get(key_).var_,i);
}

template <class Char>
template <class Key>
const typename basic_json<Char>::proxy<size_t> basic_json<Char>::proxy<Key>::operator[](size_t i) const
{
    return proxy<size_t>(var_->get(key_).var_,i);
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::proxy<Key>::operator[](std::basic_string<Char> name)
{
    return proxy<std::basic_string<Char>>(var_->get(key_).var_,name);
}

template <class Char>
template <class Key>
typename const basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::proxy<Key>::operator[](std::basic_string<Char> name) const
{
    return proxy<std::basic_string<Char>>(var_->get(key_).var_,name);
}

template <class Char>
template <class Key>
std::basic_string<Char> basic_json<Char>::proxy<Key>::to_string() const
{
	std::cout << key_ << std::endl;
	var_->get(key_);
    return var_->get(key_).var_->to_string();
}


template <class Char>
basic_json<Char>::basic_json()
    : var_(new json_object<Char>())
{
}

template <class Char>
template <class Iterator>
basic_json<Char>::basic_json(Iterator begin, Iterator end)
    : var_(new json_array<Char>(begin,end))
{
}

template <class Char>
basic_json<Char>::basic_json(const basic_json<Char>& val)
    : var_(val.var_->clone())
{
}

template <class Char>
basic_json<Char>::basic_json(basic_json&& other)
    : var_(other.var_)
{
    other.var_ = 0;
}

template <class Char>
basic_json<Char>::basic_json(json_variant<Char>* var)
    : var_(var)
{
}

template <class Char>
basic_json<Char>::basic_json(double val)
    : var_(new json_double<Char>(val))
{
}

template <class Char>
basic_json<Char>::basic_json(integer_type val)
    : var_(new json_integer<Char>(val))
{
}

template <class Char>
basic_json<Char>::basic_json(uinteger_type val)
    : var_(new json_uinteger<Char>(val))
{
}

template <class Char>
basic_json<Char>::basic_json(bool val)
    : var_(new json_bool<Char>(val))
{
}

template <class Char>
basic_json<Char>::basic_json(std::string s)
    : var_(new json_string<Char>(s))
{
}

template <class Char>
basic_json<Char>::~basic_json()
{
    delete var_;
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
    std::swap(var_,o.var_);
}

template <class Char>
size_t basic_json<Char>::size() const
{
    return var_->size();
}

template <class Char>
bool basic_json<Char>::is_null() const
{
    return var_->is_null();
}

template <class Char>
typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::operator[](std::basic_string<Char> name)
{
    return proxy<std::basic_string<Char>>(var_->object_cast(),name);
}

template <class Char>
const typename basic_json<Char>::proxy<std::basic_string<Char>> basic_json<Char>::operator[](std::basic_string<Char> name) const
{
    return proxy<std::basic_string<Char>>(var_->object_cast(),name);
}

template <class Char>
typename basic_json<Char>::proxy<size_t> basic_json<Char>::operator[](size_t i)
{
    return proxy<size_t>(var_->object_cast(),name);
}

template <class Char>
const typename basic_json<Char>::proxy<size_t> basic_json<Char>::operator[](size_t i) const
{
    return proxy<size_t>(var_->object_cast(),name);
}

template <class Char>
std::basic_string<Char> basic_json<Char>::to_string() const
{
    return var_->to_string();
}

template <class Char>
void basic_json<Char>::to_stream(std::ostream& os) const
{
    var_->to_stream(os);
}

template <class Char>
const basic_json<Char> basic_json<Char>::object_prototype(new json_object<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::array_prototype(new json_array<Char>());

template <class Char>
const basic_json<Char> basic_json<Char>::null_prototype(new json_null<Char>());

template <class Char>
basic_json<Char> basic_json<Char>::parse(std::basic_istream<Char>& is)
{
    json_parser<Char> parser;
    json_variant<Char>* var = parser.parse(is);
    return basic_json<Char>(var);
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    json_parser<Char> parser;
    json_variant<Char>* var = parser.parse(is);
    return basic_json<Char>(var);
}

template <class Char>
double basic_json<Char>::as_double() const
{
    return var_->as_double();
}

template <class Char>
int basic_json<Char>::as_int() const
{
    return var_->as_int();
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
