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
        string_value(value);
    }

    void value(double value)
    {
        double_value(value);
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

    void value(nullptr_t)
    {
        null_value();
    }

//  Implementation start here

    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void name(const std::basic_string<Char>& name) = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;

private:

    virtual void string_value(const std::basic_string<Char>& value) = 0;

    virtual void double_value(double value) = 0;

    virtual void longlong_value(long long value) = 0;

    virtual void ulonglong_value(unsigned long long value) = 0;

    virtual void bool_value(bool value) = 0;

    virtual void null_value() = 0;
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
private:

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

    virtual void null_value()
    {
    }

};

typedef basic_null_json_output_handler<char> null_json_output_handler;

typedef basic_json_output_handler<char> json_output_handler;

}
#endif
