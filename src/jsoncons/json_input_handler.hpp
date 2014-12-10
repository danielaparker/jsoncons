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

    void value(std::basic_string<Char>& value, const basic_parsing_context<Char>& context) 
    {
        write_string(&value[0], value.length(), context);
    }

    void value(const Char* value, size_t length, const basic_parsing_context<Char>& context) 
    {
        write_string(value, length, context);
    }

    void value(null_type, const basic_parsing_context<Char>& context)
    {
        write_null(context);
    }

    void value(float value, const basic_parsing_context<Char>& context)
    {
        write_double((double)value, context);
    }

    void value(double value, const basic_parsing_context<Char>& context)
    {
        write_double(value, context);
    }

    void value(long long value, const basic_parsing_context<Char>& context) 
    {
        write_longlong(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<Char>& context) 
    {
        write_ulonglong(value,context);
    }

    void value(long value, const basic_parsing_context<Char>& context) 
    {
        write_longlong((long)value,context);
    }

    void value(unsigned long value, const basic_parsing_context<Char>& context) 
    {
        write_ulonglong((unsigned long)value,context);
    }

    void value(int value, const basic_parsing_context<Char>& context) 
    {
        write_longlong((int)value,context);
    }

    void value(unsigned int value, const basic_parsing_context<Char>& context) 
    {
        write_ulonglong((unsigned int)value,context);
    }

    void value(bool value, const basic_parsing_context<Char>& context) 
    {
        write_bool(value,context);
    }

    void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        write_name(&name[0], name.length(), context);
    }

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void write_name(const Char* name, size_t length, const basic_parsing_context<Char>& context) = 0;

// value(...) implementation

    virtual void write_null(const basic_parsing_context<Char>& context) = 0;

    virtual void write_string(const Char* value, size_t length, const basic_parsing_context<Char>& context) = 0;

    virtual void write_double(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void write_longlong(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void write_ulonglong(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void write_bool(bool value, const basic_parsing_context<Char>& context) = 0;
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

    virtual void begin_object(const basic_parsing_context<Char>&)
    {
    }

    virtual void end_object(const basic_parsing_context<Char>&)
    {
    }

    virtual void begin_array(const basic_parsing_context<Char>&)
    {
    }

    virtual void end_array(const basic_parsing_context<Char>&)
    {
    }

    virtual void name(const std::basic_string<Char>&, const basic_parsing_context<Char>&)
    {
    }

    virtual void write_null(const basic_parsing_context<Char>&)
    {
    }
// value(...) implementation
    virtual void write_string(const Char*, size_t length, const basic_parsing_context<Char>&)
    {
    }

    virtual void write_double(double, const basic_parsing_context<Char>&)
    {
    }

    virtual void write_longlong(long long, const basic_parsing_context<Char>&)
    {
    }

    virtual void write_ulonglong(unsigned long long, const basic_parsing_context<Char>&)
    {
    }

    virtual void write_bool(bool, const basic_parsing_context<Char>&)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef null_basic_json_input_handler<char> null_json_input_handler;
typedef null_basic_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
