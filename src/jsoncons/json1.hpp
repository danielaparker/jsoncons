// Copyright 2013 Daniel Parker
// Distributed under Boost license

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

template <class Char>
class json_object;

template <class Char>
class json_array;

template <class Char>
struct simple_string
{
    size_t length_;
    Char* data_;
};

template <class Char>
std::basic_string<Char> escape_string(const std::basic_string<Char>& s);

template <class Char>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const simple_string<Char>& o)
{
    os << escape_string<Char>(std::basic_string<Char>(o.data_,o.length_));
    return os;
}

template <class Char>
class basic_json
{
public:
    enum value_type {object_t,array_t,string_t,double_t,longlong_t,ulonglong_t,bool_t,null_t};

    static const basic_json<Char> object_prototype;
    static const basic_json<Char> array_prototype;
    static const basic_json<Char> null;

    typedef Char char_type;

    struct proxy
    {
    	proxy(basic_json<Char>& var, std::basic_string<Char> key);

        size_t size() const;

        bool is_null() const
        {
            return val_.get(key_).is_null();
        }

        bool is_empty() const
        {
            return val_.get(key_).is_empty();
        }

        std::basic_string<Char> as_string() const;

        bool as_bool() const;

        double as_double() const;

        int as_int() const;

        unsigned int as_uint() const;

        long long as_longlong() const;

        unsigned long long as_ulonglong() const;

        operator const basic_json() const;

        proxy& operator=(const basic_json& val);

        basic_json<Char>& operator[](size_t i);

        const basic_json<Char>& operator[](size_t i) const;

        proxy operator[](std::basic_string<Char> name);

        const proxy operator[](std::basic_string<Char> name) const;

        std::basic_string<Char> to_string() const;

        basic_json<Char>& val_;

        std::basic_string<Char> key_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse(const std::basic_string<Char>& s);

    explicit basic_json();

    basic_json(const basic_json& val);

    basic_json(basic_json&& val);

    basic_json(nullptr_t);

    basic_json(double val);

    basic_json(int val);

    basic_json(unsigned int val);

    basic_json(long val);

    basic_json(unsigned long val);

    basic_json(long long val);

    basic_json(unsigned long long val);

    basic_json(const std::basic_string<Char>& val);

    basic_json(bool val);

    template <class InputIterator>
    basic_json(InputIterator first, InputIterator last);

    explicit basic_json(json_object<Char>* var);

    explicit basic_json(json_array<Char>* var);

    ~basic_json();

    basic_json& operator=(basic_json<Char> rhs);

    size_t size() const;

    basic_json<Char>& operator[](size_t i);

    const basic_json<Char>& operator[](size_t i) const;

    proxy operator[](std::basic_string<Char> k);

    const proxy operator[](std::basic_string<Char> k) const;

    std::basic_string<Char> to_string() const;

    void to_stream(std::basic_ostream<Char>& os) const;

    bool is_null() const
    {
        return type_ == null_t;
    }

    bool is_string() const
    {
        return type_ == string_t;
    }

    bool is_numeric() const
    {
        return type_ == double_t || type_ == longlong_t || type_ == ulonglong_t;
    }

    bool is_bool() const
    {
        return type_ == bool_t;
    }

    bool is_object() const
    {
        return type_ == object_t;
    }

    bool is_array() const
    {
        return type_ == array_t;
    }

    bool is_empty() const;

    bool as_bool() const;

    double as_double() const;

    int as_int() const;

    unsigned int as_uint() const;

    long long as_longlong() const;

    unsigned long long as_ulonglong() const;

    std::basic_string<Char> as_string() const;

    void swap(basic_json<Char>& o) throw();

    basic_json<Char>& at(size_t i);

    const basic_json<Char>& at(size_t i) const;

    basic_json<Char>& get(const std::basic_string<Char>& k);

    const basic_json<Char>& get(const std::basic_string<Char>& k) const;

    void set_member(const std::basic_string<Char>& k, const basic_json<Char>& value);

    void set_member(std::basic_string<Char>&& k, basic_json<Char>&& value);

    void push_back(const basic_json<Char>& value);

    void push_back(basic_json<Char>&& value);

    value_type type() const
    {
        return type_;
    }

    template <class Serializer>
    void serialize(Serializer& serializer) const;

private:
    value_type type_;
    union
    {
        double double_value_;
        long long longlong_value_;
        unsigned long long ulonglong_value_;
        bool bool_value_;
        simple_string<Char> string_value_;
        json_object<Char>* object_;
        json_array<Char>* array_;
    } value_;
};


}

#endif
