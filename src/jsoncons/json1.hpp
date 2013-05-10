#ifndef JSONCONS_JSON1_HPP
#define JSONCONS_JSON1_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include "jsoncons/json_exception.hpp"

namespace jsoncons {

class json_variant;
class json_object;

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
    	object_proxy(json_object* var, const std::basic_string<char_type>& name);

        operator const basic_json() const;

        operator const double() const;

        object_proxy& operator=(const char_type* val);

        object_proxy& operator=(std::basic_string<char_type> val);

        object_proxy& operator=(double val);

        object_proxy& operator=(int val);

        object_proxy& operator=(bool val);

        object_proxy& operator=(const basic_json& val);

        object_proxy operator[](const std::basic_string<char_type>& name);

        json_object* var_;
        const std::basic_string<char_type>& name_;
    };

    static basic_json parse(std::basic_istream<char_type>& is);

    static basic_json parse(const std::basic_string<char_type>& s);

    basic_json();

    basic_json(const basic_json& val);

    basic_json(json_variant* var);

    ~basic_json();

    basic_json& operator=(const basic_json& rhs);

    size_t size() const;

    bool is_null() const;

    object_proxy operator[](const std::basic_string<char_type>& name);

    std::basic_string<char_type> to_string() const;

    json_variant* var_;
};


}

#endif
