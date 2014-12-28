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
class basic_parse_context;

template <typename Char>
class basic_json_input_handler
{
public:
    virtual ~basic_json_input_handler() {}

    void begin_json()
    {
        do_begin_json();
    }

    void end_json()
    {
        do_end_json();
    }

    void begin_object(basic_parse_context<Char> context)
    {
        do_begin_object(context);
    }

    void end_object(basic_parse_context<Char> context)
    {
        do_end_object(context);
    }

    void begin_array(basic_parse_context<Char> context)
    {
        do_begin_array(context);
    }

    void end_array(basic_parse_context<Char> context)
    {
        do_end_array(context);
    }

    void name(const std::basic_string<Char>& name, basic_parse_context<Char> context)
    {
        do_name(&name[0], name.length(), context);
    }

    void name(const Char* p, size_t length, basic_parse_context<Char> context) 
    {
        do_name(p, length, context);
    }

    void value(const std::basic_string<Char>& value, basic_parse_context<Char> context) 
    {
        do_string_value(&value[0], value.length(), context);
    }

    void value(const Char* p, size_t length, basic_parse_context<Char> context) 
    {
        do_string_value(p, length, context);
    }

    void value(int value, basic_parse_context<Char> context) 
    {
        do_longlong_value(value,context);
    }

    void value(long value, basic_parse_context<Char> context) 
    {
        do_longlong_value(value,context);
    }

    void value(long long value, basic_parse_context<Char> context) 
    {
        do_longlong_value(value,context);
    }

    void value(unsigned int value, basic_parse_context<Char> context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(unsigned long value, basic_parse_context<Char> context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(unsigned long long value, basic_parse_context<Char> context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(float value, basic_parse_context<Char> context)
    {
        do_double_value(value, context);
    }

    void value(double value, basic_parse_context<Char> context)
    {
        do_double_value(value, context);
    }

    void value(bool value, basic_parse_context<Char> context) 
    {
        do_bool_value(value,context);
    }

    void value(null_type, basic_parse_context<Char> context)
    {
        do_null_value(context);
    }

private:
    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_begin_object(basic_parse_context<Char> context) = 0;

    virtual void do_end_object(basic_parse_context<Char> context) = 0;

    virtual void do_begin_array(basic_parse_context<Char> context) = 0;

    virtual void do_end_array(basic_parse_context<Char> context) = 0;

    virtual void do_name(const Char* name, size_t length, basic_parse_context<Char> context) = 0;

    virtual void do_null_value(basic_parse_context<Char> context) = 0;

    virtual void do_string_value(const Char* value, size_t length, basic_parse_context<Char> context) = 0;

    virtual void do_double_value(double value, basic_parse_context<Char> context) = 0;

    virtual void do_longlong_value(long long value, basic_parse_context<Char> context) = 0;

    virtual void do_ulonglong_value(unsigned long long value, basic_parse_context<Char> context) = 0;

    virtual void do_bool_value(bool value, basic_parse_context<Char> context) = 0;
};


template <typename Char>
class empty_basic_json_input_handler : public basic_json_input_handler<Char>
{
public:
    static basic_json_input_handler<Char>& instance()
    {
        static empty_basic_json_input_handler<Char> instance;
        return instance;
    }
private:
    virtual void do_begin_json()
    {
    }

    virtual void do_end_json()
    {
    }

    virtual void do_begin_object(basic_parse_context<Char>)
    {
    }

    virtual void do_end_object(basic_parse_context<Char>)
    {
    }

    virtual void do_begin_array(basic_parse_context<Char>)
    {
    }

    virtual void do_end_array(basic_parse_context<Char>)
    {
    }

    virtual void do_name(const Char* p, size_t length, basic_parse_context<Char>)
    {
    }

    virtual void do_null_value(basic_parse_context<Char>)
    {
    }

    virtual void do_string_value(const Char*, size_t length, basic_parse_context<Char>)
    {
    }

    virtual void do_double_value(double, basic_parse_context<Char>)
    {
    }

    virtual void do_longlong_value(long long, basic_parse_context<Char>)
    {
    }

    virtual void do_ulonglong_value(unsigned long long, basic_parse_context<Char>)
    {
    }

    virtual void do_bool_value(bool, basic_parse_context<Char>)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef empty_basic_json_input_handler<char> empty_json_input_handler;
typedef empty_basic_json_input_handler<wchar_t> wempty_json_input_handler;

}

#endif
