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

class json_value
{
public:
    static const json_value object_prototype;
    static const json_value array_prototype;
    static const json_value null_prototype;

    struct object_proxy
    {
        object_proxy(json_object* var, const std::string& name)
            : var_(var), name_(name)
        {
        }

        operator const json_value() const
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return json_value(var_->clone());
        }

        operator const double() const
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return v->as_double();
        }

        object_proxy& operator=(const char* val)
        {
            var_->set_member(name_, new json_string(val));
            return *this;
        }

        object_proxy& operator=(std::string val)
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

        object_proxy& operator=(const json_value& val)
        {
            var_->set_member(name_, val.var_->clone());
            return *this;
        }

        object_proxy operator[](const std::string& name)
        {
            json_variant* v = var_->get(name_);
            JSONCONS_ASSERT(v != 0);
            return object_proxy(v->object_cast(),name);
        }

        json_object* var_;
        const std::string& name_;
    };

    static json_value parse(std::istream& is);

    static json_value parse(const std::string& s);

    json_value()
        : var_(new json_object())
    {
    }

    json_value(const json_value& val)
        : var_(val.var_->clone())
    {
    }

    json_value(json_variant* var)
        : var_(var)
    {
    }

    ~json_value()
    {
        delete var_;
    }

    json_value& operator=(const json_value& rhs)
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

    object_proxy operator[](const std::string& name)
    {
        return object_proxy(var_->object_cast(),name);
    }

    std::string to_string() const
    {
        return var_->to_string();
    }

    json_variant* var_;
};

}

#endif
