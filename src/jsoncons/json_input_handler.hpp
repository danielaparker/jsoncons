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
    virtual void begin_json() = 0;

    void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) 
    {
        string_value(value, context);
    }

    void value(null_type, const basic_parsing_context<Char>& context)
    {
        null_value(context);
    }

    void value(float value, const basic_parsing_context<Char>& context)
    {
        double_value((double)value, context);
    }

    void value(double value, const basic_parsing_context<Char>& context)
    {
        double_value(value, context);
    }

    void value(long long value, const basic_parsing_context<Char>& context) 
    {
        longlong_value(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<Char>& context) 
    {
        ulonglong_value(value,context);
    }

    void value(long value, const basic_parsing_context<Char>& context) 
    {
        longlong_value((long)value,context);
    }

    void value(unsigned long value, const basic_parsing_context<Char>& context) 
    {
        ulonglong_value((unsigned long)value,context);
    }

    void value(int value, const basic_parsing_context<Char>& context) 
    {
        longlong_value((int)value,context);
    }

    void value(unsigned int value, const basic_parsing_context<Char>& context) 
    {
        ulonglong_value((unsigned int)value,context);
    }

    void value(bool value, const basic_parsing_context<Char>& context) 
    {
        bool_value(value,context);
    }

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context) = 0;

// value(...) implementation

    virtual void null_value(const basic_parsing_context<Char>& context) = 0;

    virtual void string_value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) = 0;

    virtual void double_value(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void longlong_value(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void bool_value(bool value, const basic_parsing_context<Char>& context) = 0;
};

template <typename Char>
class null_basic_json_input_handler : public basic_json_input_handler<Char>
{
public:
    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<Char>& context)
    {
    }

    virtual void end_object(const basic_parsing_context<Char>& context)
    {
    }

    virtual void begin_array(const basic_parsing_context<Char>& context)
    {
    }

    virtual void end_array(const basic_parsing_context<Char>& context)
    {
    }

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
    }

    virtual void null_value(const basic_parsing_context<Char>& context)
    {
    }
// value(...) implementation
    virtual void string_value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void double_value(double value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void longlong_value(long long value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void bool_value(bool value, const basic_parsing_context<Char>& context)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef null_basic_json_input_handler<char> null_json_input_handler;
typedef null_basic_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
