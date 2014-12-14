// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template <typename Char>
class basic_parsing_context;

template <typename Char>
class basic_json_input_handler
{
public:
    virtual ~basic_json_input_handler() {}
    virtual void do_begin_json() = 0;

    void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        do_name(&name[0], name.length(), context);
    }

    void name(const Char* p, size_t length, const basic_parsing_context<Char>& context) 
    {
        do_name(p, length, context);
    }

    void string_value(std::basic_string<Char>& value, const basic_parsing_context<Char>& context) 
    {
        do_string_value(&value[0], value.length(), context);
    }

    void string_value(const Char* p, size_t length, const basic_parsing_context<Char>& context) 
    {
        do_string_value(p, length, context);
    }

    void longlong_value(long long value, const basic_parsing_context<Char>& context) 
    {
        do_longlong_value(value,context);
    }

    void ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context) 
    {
        do_ulonglong_value(value,context);
    }

    void double_value(double value, const basic_parsing_context<Char>& context)
    {
        do_double_value(value, context);
    }

    void bool_value(bool value, const basic_parsing_context<Char>& context) 
    {
        do_bool_value(value,context);
    }

    void null_value(const basic_parsing_context<Char>& context)
    {
        do_null_value(context);
    }

    virtual void do_end_json() = 0;

    virtual void do_begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void do_end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void do_begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void do_end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void do_name(const Char* name, size_t length, const basic_parsing_context<Char>& context) = 0;

// value(...) implementation

    virtual void do_null_value(const basic_parsing_context<Char>& context) = 0;

    virtual void do_string_value(const Char* value, size_t length, const basic_parsing_context<Char>& context) = 0;

    virtual void do_double_value(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_longlong_value(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_bool_value(bool value, const basic_parsing_context<Char>& context) = 0;
};

template <typename Char>
class null_basic_json_input_handler : public basic_json_input_handler<Char>
{
public:
    virtual void do_begin_json()
    {
    }

    virtual void do_end_json()
    {
    }

    virtual void do_begin_object(const basic_parsing_context<Char>&)
    {
    }

    virtual void do_end_object(const basic_parsing_context<Char>&)
    {
    }

    virtual void do_begin_array(const basic_parsing_context<Char>&)
    {
    }

    virtual void do_end_array(const basic_parsing_context<Char>&)
    {
    }

    virtual void name(const std::basic_string<Char>&, const basic_parsing_context<Char>&)
    {
    }

    virtual void do_null_value(const basic_parsing_context<Char>&)
    {
    }
// value(...) implementation
    virtual void do_string_value(const Char*, size_t length, const basic_parsing_context<Char>&)
    {
    }

    virtual void do_double_value(double, const basic_parsing_context<Char>&)
    {
    }

    virtual void do_longlong_value(long long, const basic_parsing_context<Char>&)
    {
    }

    virtual void do_ulonglong_value(unsigned long long, const basic_parsing_context<Char>&)
    {
    }

    virtual void do_bool_value(bool, const basic_parsing_context<Char>&)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef null_basic_json_input_handler<char> null_json_input_handler;
typedef null_basic_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
