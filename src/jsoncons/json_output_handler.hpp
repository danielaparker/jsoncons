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

    // value interface
    void value(const std::basic_string<Char>& value) 
    {
        value_string(value);
    }

    void value(double value)
    {
        value_double(value);
    }

    void value(long long value) 
    {
        value_longlong(value);
    }

    void value(unsigned long long value)
    {
        value_ulonglong(value);
    }

    void value(int value) 
    {
        value_longlong((long long)value);
    }

    void value(unsigned int value)
    {
        value_ulonglong((unsigned long long)value);
    }

    void value(long value) 
    {
        value_longlong((long long)value);
    }

    void value(unsigned long value)
    {
        value_ulonglong((unsigned long long)value);
    }

    void value(bool value)
    {
        value_bool(value);
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

protected:

    virtual void value_string(const std::basic_string<Char>& value) = 0;

    virtual void value_double(double value) = 0;

    virtual void value_longlong(long long value) = 0;

    virtual void value_ulonglong(unsigned long long value) = 0;

    virtual void value_bool(bool value) = 0;
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

protected:

    virtual void value_string(const std::basic_string<Char>& value)
    {
    }

    virtual void value_double(double value)
    {
    }

    virtual void value_longlong(long long value)
    {
    }

    virtual void value_ulonglong(unsigned long long value)
    {
    }

    virtual void value_bool(bool value)
    {
    }

};

typedef basic_null_json_output_handler<char> null_json_output_handler;

typedef basic_json_output_handler<char> json_output_handler;

}
#endif
