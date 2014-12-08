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
        write_string(value, length, context);
    }

    void value(null_type, const basic_parsing_context<CharT>& context)
    {
        write_null(context);
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
        write_longlong(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<CharT>& context) 
    {
        write_ulonglong(value,context);
    }

    void value(long value, const basic_parsing_context<CharT>& context) 
    {
        write_longlong((long)value,context);
    }

    void value(unsigned long value, const basic_parsing_context<CharT>& context) 
    {
        write_ulonglong((unsigned long)value,context);
    }

    void value(int value, const basic_parsing_context<CharT>& context) 
    {
        write_longlong((int)value,context);
    }

    void value(unsigned int value, const basic_parsing_context<CharT>& context) 
    {
        write_ulonglong((unsigned int)value,context);
    }

    void value(bool value, const basic_parsing_context<CharT>& context) 
    {
        write_bool(value,context);
    }

    void name(const std::basic_string<CharT>& name, const basic_parsing_context<CharT>& context)
    {
        write_name(name,context);
    }

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void end_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void begin_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void end_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void write_name(const std::basic_string<CharT>& name, const basic_parsing_context<CharT>& context) = 0;

// value(...) implementation

    virtual void write_null(const basic_parsing_context<CharT>& context) = 0;

    virtual void write_string(const CharT* value, size_t length, const basic_parsing_context<CharT>& context) = 0;

    virtual void double_value(double value, const basic_parsing_context<CharT>& context) = 0;

    virtual void write_longlong(long long value, const basic_parsing_context<CharT>& context) = 0;

    virtual void write_ulonglong(unsigned long long value, const basic_parsing_context<CharT>& context) = 0;

    virtual void write_bool(bool value, const basic_parsing_context<CharT>& context) = 0;
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

    virtual void write_null(const basic_parsing_context<CharT>&)
    {
    }
// value(...) implementation
    virtual void write_string(const CharT*, size_t length, const basic_parsing_context<CharT>&)
    {
    }

    virtual void double_value(double, const basic_parsing_context<CharT>&)
    {
    }

    virtual void write_longlong(long long, const basic_parsing_context<CharT>&)
    {
    }

    virtual void write_ulonglong(unsigned long long, const basic_parsing_context<CharT>&)
    {
    }

    virtual void write_bool(bool, const basic_parsing_context<CharT>&)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef null_basic_json_input_handler<char> null_json_input_handler;
typedef null_basic_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
