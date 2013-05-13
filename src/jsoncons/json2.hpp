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

template <class Char>
basic_json<Char>::object_proxy::object_proxy(json_object<Char>* var, const std::basic_string<char_type>& name)
    : var_(var), name_(name)
{
}

template <class Char>
basic_json<Char>::object_proxy::operator const basic_json<Char>() const
{
    return var_->get(name_);
}

template <class Char>
basic_json<Char>::object_proxy::operator const double() const
{
    return var_->get(name_).as_double();
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(const char_type* val)
{
    var_->set_member(name_, new json_string<Char>(val));
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(std::basic_string<char_type> val)
{
    var_->set_member(name_, new json_string(val));
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(double val)
{
    var_->set_member(name_, new json_double<Char>(val));
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(int val)
{
    var_->set_member(name_, new json_long<Char>(val));
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(bool val)
{
    var_->set_member(name_, new json_bool<Char>(val));
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy& basic_json<Char>::object_proxy::operator=(const basic_json& val)
{
    var_->set_member(name_, val.var_->clone());
    return *this;
}

template <class Char>
typename basic_json<Char>::object_proxy basic_json<Char>::object_proxy::operator[](const std::basic_string<char_type>& name)
{
    return object_proxy(static_cast<json_object<Char>*>(var_),name);
}


template <class Char>
basic_json<Char>::basic_json()
    : var_(new json_object<Char>())
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
    return 0;
}

template <class Char>
bool basic_json<Char>::is_null() const
{
    return var_->is_null();
}

template <class Char>
typename basic_json<Char>::object_proxy basic_json<Char>::operator[](const std::basic_string<char_type>& name)
{
    return object_proxy(var_->object_cast(),name);
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
basic_json<Char> basic_json<Char>::parse(std::basic_istream<char_type>& is)
{
    json_parser<Char> parser;
    json_variant<Char>* object = parser.parse(is);
    return basic_json<Char>(object);
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<char_type>& s)
{
    std::basic_istringstream<char_type> is(s);
    json_parser<Char> parser;
    json_variant<Char>* object = parser.parse(is);
    return basic_json<Char>(object);
}

template <class Char>
double basic_json<Char>::as_double() const
{
    return var_->as_double();
}

}

#endif
