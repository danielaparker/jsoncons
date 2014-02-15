// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_OUTPUT_HANDLER_HPP
#define JSONCONS_JSON_OUTPUT_HANDLER_HPP

#include <string>

namespace jsoncons {

template <class Char>
class basic_json_output_handler
{
public:
    virtual ~basic_json_output_handler() {}

    // Overloaded methods
    void value(const std::basic_string<Char>& value) 
    {
        write_value(value);
    }

    void value(float value)
    {
        write_value((double)value);
    }

    void value(double value)
    {
        write_value(value);
    }

    void value(int value) 
    {
        write_value((long long)value);
    }

    void value(unsigned int value)
    {
        write_value((unsigned long long)value);
    }

    void value(long value) 
    {
        write_value((long long)value);
    }

    void value(unsigned long value)
    {
        write_value((unsigned long long)value);
    }

    void value(long long value) 
    {
        write_value(value);
    }

    void value(unsigned long long value)
    {
        write_value(value);
    }

    void value(bool value)
    {
        write_value(value);
    }

    virtual void null_value() = 0;

//  Implementation start here

    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void name(const std::basic_string<Char>& name) = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;

// value(...) implementation

    virtual void write_value(const std::basic_string<Char>& value) = 0;

    virtual void write_value(double value) = 0;

    virtual void write_value(long long value) = 0;

    virtual void write_value(unsigned long long value) = 0;

    virtual void write_value(bool value) = 0;
};

template <class Char>
class basic_null_json_output_handler : public basic_json_output_handler<Char>
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

    virtual void write_value(const std::basic_string<Char>& value)
    {
    }

    virtual void write_value(double value)
    {
    }

    virtual void write_value(long long value)
    {
    }

    virtual void write_value(unsigned long long value)
    {
    }

    virtual void write_value(bool value)
    {
    }

};

typedef basic_null_json_output_handler<char> null_json_output_handler;

typedef basic_json_output_handler<char> json_output_handler;

}
#endif
