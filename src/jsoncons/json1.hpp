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
class json_variant;

template <class Char>
class json_object;

template <class Char>
class json_array;

template <class Char>
class basic_json
{
public:
    static const basic_json<Char> object_prototype;
    static const basic_json<Char> array_prototype;
    static const basic_json<Char> null_prototype;

    typedef Char char_type;

    template <class Key>
    struct proxy
    {
    	proxy(json_variant<Char>* var, Key key);

        size_t size() const;

        operator const basic_json() const;

        proxy<Key>& operator=(const Char* val);

        proxy<Key>& operator=(std::basic_string<Char> val);

        proxy<Key>& operator=(double val);

        proxy<Key>& operator=(int val);

        proxy<Key>& operator=(bool val);

        proxy<Key>& operator=(const basic_json& val);

        proxy<size_t> operator[](size_t i);

        const proxy<size_t> operator[](size_t i) const;

        proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name);

        const proxy<std::basic_string<Char>> operator[](std::basic_string<Char> name) const;

        std::basic_string<Char> to_string() const;

        json_variant<Char>* var_;

        Key key_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse(const std::basic_string<Char>& s);

    basic_json();

    basic_json(const basic_json& val);

    template <class Iterator>
    basic_json(Iterator begin, Iterator end);

    explicit basic_json(json_variant<Char>* var);

    explicit basic_json(basic_json&& other);

    ~basic_json();

    basic_json& operator=(const basic_json& rhs);

    basic_json& operator=(basic_json<Char>&& other);

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

    json_variant<Char>* var_;
};


}

#endif
