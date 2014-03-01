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
        string_value(value);
    }

    void value(null_type)
    {
        null_value();
    }

    void value(float value)
    {
        double_value((double)value);
    }

    void value(double value)
    {
        double_value(value);
    }

    void value(int value) 
    {
        longlong_value((long long)value);
    }

    void value(unsigned int value)
    {
        ulonglong_value((unsigned long long)value);
    }

    void value(long value) 
    {
        longlong_value((long long)value);
    }

    void value(unsigned long value)
    {
        ulonglong_value((unsigned long long)value);
    }

    void value(long long value) 
    {
        longlong_value(value);
    }

    void value(unsigned long long value)
    {
        ulonglong_value(value);
    }

    void value(bool value)
    {
        bool_value(value);
    }

//  Implementation start here

    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void name(const std::basic_string<Char>& name) = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;

// value(...) implementation

    virtual void null_value() = 0;

    virtual void string_value(const std::basic_string<Char>& value) = 0;

    virtual void double_value(double value) = 0;

    virtual void longlong_value(long long value) = 0;

    virtual void ulonglong_value(unsigned long long value) = 0;

    virtual void bool_value(bool value) = 0;
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

    virtual void name(const std::basic_string<Char>& name)
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

    virtual void null_value()
    {
    }

// value(...) implementation

    virtual void string_value(const std::basic_string<Char>& value)
    {
    }

    virtual void double_value(double value)
    {
    }

    virtual void longlong_value(long long value)
    {
    }

    virtual void ulonglong_value(unsigned long long value)
    {
    }

    virtual void bool_value(bool value)
    {
    }

};

typedef basic_json_output_handler<char> json_output_handler;
typedef basic_json_output_handler<wchar_t> wjson_output_handler;

typedef null_basic_json_output_handler<char> null_json_output_handler;
typedef null_basic_json_output_handler<wchar_t> wnull_json_output_handler;

}
#endif
