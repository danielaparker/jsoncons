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

template <typename CharT>
class basic_parsing_context;

template <typename CharT>
class basic_json_input_handler
{
public:
    virtual ~basic_json_input_handler() {}
    virtual void begin_json() = 0;

    void value(const CharT* value, size_t length, const basic_parsing_context<CharT>& context) 
    {
        string_value(value, length, context);
    }

    void value(null_type, const basic_parsing_context<CharT>& context)
    {
        null_value(context);
    }

    void value(float value, const basic_parsing_context<CharT>& context)
    {
        double_value((double)value, context);
    }

    void value(double value, const basic_parsing_context<CharT>& context)
    {
        double_value(value, context);
    }

    void value(long long value, const basic_parsing_context<CharT>& context) 
    {
        longlong_value(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<CharT>& context) 
    {
        ulonglong_value(value,context);
    }

    void value(long value, const basic_parsing_context<CharT>& context) 
    {
        longlong_value((long)value,context);
    }

    void value(unsigned long value, const basic_parsing_context<CharT>& context) 
    {
        ulonglong_value((unsigned long)value,context);
    }

    void value(int value, const basic_parsing_context<CharT>& context) 
    {
        longlong_value((int)value,context);
    }

    void value(unsigned int value, const basic_parsing_context<CharT>& context) 
    {
        ulonglong_value((unsigned int)value,context);
    }

    void value(bool value, const basic_parsing_context<CharT>& context) 
    {
        bool_value(value,context);
    }

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void end_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void begin_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void end_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void name(const std::basic_string<CharT>& name, const basic_parsing_context<CharT>& context) = 0;

// value(...) implementation

    virtual void null_value(const basic_parsing_context<CharT>& context) = 0;

    virtual void string_value(const CharT* value, size_t length, const basic_parsing_context<CharT>& context) = 0;

    virtual void double_value(double value, const basic_parsing_context<CharT>& context) = 0;

    virtual void longlong_value(long long value, const basic_parsing_context<CharT>& context) = 0;

    virtual void ulonglong_value(unsigned long long value, const basic_parsing_context<CharT>& context) = 0;

    virtual void bool_value(bool value, const basic_parsing_context<CharT>& context) = 0;
};

template <typename CharT>
class null_basic_json_input_handler : public basic_json_input_handler<CharT>
{
public:
    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<CharT>&)
    {
    }

    virtual void end_object(const basic_parsing_context<CharT>&)
    {
    }

    virtual void begin_array(const basic_parsing_context<CharT>&)
    {
    }

    virtual void end_array(const basic_parsing_context<CharT>&)
    {
    }

    virtual void name(const std::basic_string<CharT>&, const basic_parsing_context<CharT>&)
    {
    }

    virtual void null_value(const basic_parsing_context<CharT>&)
    {
    }
// value(...) implementation
    virtual void string_value(const CharT*, size_t length, const basic_parsing_context<CharT>&)
    {
    }

    virtual void double_value(double, const basic_parsing_context<CharT>&)
    {
    }

    virtual void longlong_value(long long, const basic_parsing_context<CharT>&)
    {
    }

    virtual void ulonglong_value(unsigned long long, const basic_parsing_context<CharT>&)
    {
    }

    virtual void bool_value(bool, const basic_parsing_context<CharT>&)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef null_basic_json_input_handler<char> null_json_input_handler;
typedef null_basic_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
