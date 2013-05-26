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

typedef long long integer_type;
typedef unsigned long long uinteger_type;

template <class Char>
class json_variant;

template <class Char>
class json_object;

template <class Char>
class json_array;

template <class Char>
union value_union
{
public:
    double double_value_;
    json_variant<Char>* var_;
};

template <class Char>
class basic_json
{
public:
    enum value_type {object_t,string_t,double_t,long_t,ulong_t,array_t,bool_t,null_t};

    static const basic_json<Char> object_prototype;
    static const basic_json<Char> array_prototype;
    static const basic_json<Char> null_prototype;

    typedef Char char_type;

    template <class Key>
    struct proxy
    {
    	proxy(basic_json<Char>& var, Key key);

        size_t size() const;

        operator const basic_json() const;

        proxy<Key>& operator=(const basic_json& val);

        proxy<size_t> operator[](size_t i);

        const proxy<size_t> operator[](size_t i) const;

        proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name);

        const proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name) const;

        std::basic_string<Char> to_string() const;

        basic_json<Char>& var_;

        Key key_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse(const std::basic_string<Char>& s);

    basic_json();

    basic_json(const basic_json& val);

    explicit basic_json(nullptr_t nullp);

    explicit basic_json(double val);

    explicit basic_json(integer_type val);

    explicit basic_json(uinteger_type val);

    explicit basic_json(std::string val);

    explicit basic_json(bool val);

    template <class Iterator>
    basic_json(Iterator begin, Iterator end);

    explicit basic_json(json_variant<Char>* var);

    explicit basic_json(basic_json<Char>&& other);

    ~basic_json();

    basic_json& operator=(basic_json<Char> rhs);

    size_t size() const;

    bool is_null() const;

    proxy<size_t> operator[](size_t);

    const proxy<size_t> operator[](size_t) const;

    proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name);

    const proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name) const;

    std::basic_string<Char> to_string() const;

    void to_stream(std::ostream& os) const;

    double as_double() const;

    int as_int() const;

    void swap(basic_json<Char>& o) throw();

    basic_json<Char>& get(size_t i);

    const basic_json<Char>& get(size_t i) const;

    basic_json<Char>& get(const std::string& name);

    const basic_json<Char>& get(const std::string& name) const;

    void set_member(const std::basic_string<Char>& name, basic_json<Char> value);

private:
    value_type type_;
    json_variant<Char>* var_;
    value_union<Char> value_;
};


}

#endif
