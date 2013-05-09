#ifndef JSONCONS_JSONVALUE_HPP
#define JSONCONS_JSONVALUE_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include "jsoncons/json_variant.hpp"

namespace jsoncons {

class json_variant;
class name_value_pair;

template <class Char>
class basic_json
{
public:
    static const basic_json<Char> object_prototype;
    static const basic_json<Char> array_prototype;
    static const basic_json<Char> null_prototype;

    typedef Char char_type;

    struct object_proxy
    {
        object_proxy(json_object* var, const std::basic_string<char_type>& name)
            : var_(var), name_(name)
        {
        }

        operator const basic_json() const
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return basic_json(var_->clone());
        }

        operator const double() const
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return v->as_double();
        }

        object_proxy& operator=(const char_type* val)
        {
            var_->set_member(name_, new json_string(val));
            return *this;
        }

        object_proxy& operator=(std::basic_string<char_type> val)
        {
            var_->set_member(name_, new json_string(val));
            return *this;
        }

        object_proxy& operator=(double val)
        {
            var_->set_member(name_, new json_double(val));
            return *this;
        }

        object_proxy& operator=(int val)
        {
            var_->set_member(name_, new json_long(val));
            return *this;
        }

        object_proxy& operator=(bool val)
        {
            var_->set_member(name_, new json_bool(val));
            return *this;
        }

        object_proxy& operator=(const basic_json& val)
        {
            var_->set_member(name_, val.var_->clone());
            return *this;
        }

        object_proxy operator[](const std::basic_string<char_type>& name)
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return object_proxy(v->object_cast(),name);
        }

        json_object* var_;
        const std::basic_string<char_type>& name_;
    };

    static basic_json parse(std::basic_istream<char_type>& is);

    static basic_json parse(const std::basic_string<char_type>& s);

    basic_json()
        : var_(new json_object())
    {
    }

    basic_json(const basic_json& val)
        : var_(val.var_->clone())
    {
    }

    basic_json(json_variant* var)
        : var_(var)
    {
    }

    ~basic_json()
    {
        delete var_;
    }

    basic_json& operator=(const basic_json& rhs)
    {
        if (&rhs != this)
        {
            delete var_;
            var_ = rhs.var_->clone();
        }
        return *this;
    }

    size_t size() const;

    bool is_null() const
    {
        return var_->is_null();
    }

    object_proxy operator[](const std::basic_string<char_type>& name)
    {
        return object_proxy(var_->object_cast(),name);
    }

    std::basic_string<char_type> to_string() const
    {
        return var_->to_string();
    }

    json_variant* var_;
};

template <class Char>
const basic_json<Char> basic_json<Char>::object_prototype(new json_object());

template <class Char>
const basic_json<Char> basic_json<Char>::array_prototype(new json_array());

template <class Char>
const basic_json<Char> basic_json<Char>::null_prototype(new json_null());


template <class Char>
basic_json<Char> basic_json<Char>::parse(std::basic_istream<char_type>& is)
{
    json_parser parser;
    json_variant* object = parser.parse(is);
    return basic_json<Char>(object);
}


template <class Char>
basic_json<Char> basic_json<Char>::parse(const std::basic_string<char_type>& s)
{
    std::basic_istringstream<char_type> is(s);
    json_parser parser;
    json_variant* object = parser.parse(is);
    return basic_json<Char>(object);
}

typedef basic_json<char> json;

}

#endif
