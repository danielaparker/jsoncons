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
basic_json<Char>::proxy<Key>::operator const double() const
{
    return var_->get(key_).as_double();
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(const Char* val)
{
    var_->set_member(key_, new json_string<Char>(val));
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(std::basic_string<Char> val)
{
    var_->set_member(key_, new json_string(val));
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(double val)
{
    var_->set_member(key_, new json_double<Char>(val));
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(int val)
{
    var_->set_member(key_, new json_long<Char>(val));
    return *this;
}

template <class Char>
template <class Key>
typename basic_json<Char>::proxy<Key>& basic_json<Char>::proxy<Key>::operator=(bool val)
{
    var_->set_member(key_, new json_bool<Char>(val));
    return *this;
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
    return proxy<size_t>(var_,i);
}

template <class Char>
template <class Key>
const typename basic_json<Char>::proxy<size_t> basic_json<Char>::proxy<Key>::operator[](size_t i) const
{
    return proxy<size_t>(var_,i);
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

// Move assignment operator.
template <class Char>
basic_json<Char>& basic_json<Char>::operator=(basic_json<Char>&& other)
{
   if (this != &other)
   {
      delete[] var_;
      var_ = other.var_;
      other.var_ = 0;
   }
   return *this;
}

template <class Char>
basic_json<Char>::basic_json(json_variant<Char>* var)
    : var_(var)
{
}

template <class Char>
basic_json<Char>::~basic_json()
{
    delete var_;
}

template <class Char>
basic_json<Char>& basic_json<Char>::operator=(const basic_json<Char>& rhs)
{
    if (&rhs != this)
    {
        delete var_;
        var_ = rhs.var_->clone();
    }
    return *this;
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
    json_variant<Char>* object = parser.parse(is);
    return basic_json<Char>(object);
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> is(s);
    json_parser<Char> parser;
    json_variant<Char>* object = parser.parse(is);
    return basic_json<Char>(object);
}

template <class Char>
double basic_json<Char>::as_double() const
{
    return var_->as_double();
}

template <class Char>
std::ostream& operator<<(std::ostream& os, const basic_json<Char>& o)
{
    os << o.to_string();
    return os;
}

}

#endif
