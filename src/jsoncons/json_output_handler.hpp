// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_OUTPUT_HANDLER_HPP
#define JSONCONS_JSON_OUTPUT_HANDLER_HPP

#include <string>

namespace jsoncons {

template <typename Char>
class basic_json_output_handler
{
public:
    virtual ~basic_json_output_handler() {}

    // Overloaded methods
    void value(const std::basic_string<Char>& value) 
    {
        write_string(&value[0], value.length());
    }

    void value(const Char* value, size_t length) 
    {
        write_string(value, length);
    }

    void value(null_type)
    {
        write_null();
    }

    void value(float value)
    {
        write_double((double)value);
    }

    void value(double value)
    {
        write_double(value);
    }

    void value(int value) 
    {
        write_longlong((long long)value);
    }

    void value(unsigned int value)
    {
        write_ulonglong((unsigned long long)value);
    }

    void value(long value) 
    {
        write_longlong((long long)value);
    }

    void value(unsigned long value)
    {
        write_ulonglong((unsigned long long)value);
    }

    void value(long long value) 
    {
        write_longlong(value);
    }

    void value(unsigned long long value)
    {
        write_ulonglong(value);
    }

    void value(bool value)
    {
        write_bool(value);
    }

    void name(const std::basic_string<Char>& name)
    {
        write_name(&name[0], name.length());
    }

//  Implementation start here

    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void write_name(const Char* name, size_t length) = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;

// value(...) implementation

    virtual void write_null() = 0;

    virtual void write_string(const Char* value, size_t length) = 0;

    virtual void write_double(double value) = 0;

    virtual void write_longlong(long long value) = 0;

    virtual void write_ulonglong(unsigned long long value) = 0;

    virtual void write_bool(bool value) = 0;
};

template <typename Char>
class null_basic_json_output_handler : public basic_json_output_handler<Char>
{
public:

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void write_name(const Char* name, size_t length)
    {
    }

    virtual void begin_object()
    {
    }

    virtual void end_object()
    {
    }

    virtual void begin_array()
    {
    }

    virtual void end_array()
    {
    }

    virtual void write_null()
    {
    }

// value(...) implementation

    virtual void write_string(const Char*, size_t length)
    {
    }

    virtual void write_double(double)
    {
    }

    virtual void write_longlong(long long)
    {
    }

    virtual void write_ulonglong(unsigned long long)
    {
    }

    virtual void write_bool(bool)
    {
    }

};

typedef basic_json_output_handler<char> json_output_handler;
typedef basic_json_output_handler<wchar_t> wjson_output_handler;

typedef null_basic_json_output_handler<char> null_json_output_handler;
typedef null_basic_json_output_handler<wchar_t> wnull_json_output_handler;

}
#endif
