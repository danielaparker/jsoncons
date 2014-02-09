// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include "jsoncons/jsoncons_config.hpp"

namespace jsoncons {

template <class Char>
class basic_parsing_context;

template <class Char>
class basic_json_input_handler
{
public:
    virtual ~basic_json_input_handler() {}
    virtual void begin_json() = 0;

    void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) 
    {
        value_string(value, context);
    }

    void value(float value, const basic_parsing_context<Char>& context)
    {
        value_double((double)value, context);
    }

    void value(double value, const basic_parsing_context<Char>& context)
    {
        value_double(value, context);
    }

    void value(long long value, const basic_parsing_context<Char>& context) 
    {
        value_longlong(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<Char>& context) 
    {
        value_ulonglong(value,context);
    }

    void value(long value, const basic_parsing_context<Char>& context) 
    {
        value_longlong((long)value,context);
    }

    void value(unsigned long value, const basic_parsing_context<Char>& context) 
    {
        value_ulonglong((unsigned long)value,context);
    }

    void value(int value, const basic_parsing_context<Char>& context) 
    {
        value_longlong((int)value,context);
    }

    void value(unsigned int value, const basic_parsing_context<Char>& context) 
    {
        value_ulonglong((unsigned int)value,context);
    }

    void value(bool value, const basic_parsing_context<Char>& context) 
    {
        value_bool(value,context);
    }

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context) = 0;

    virtual void null_value(const basic_parsing_context<Char>& context) = 0;

protected:
    virtual void value_string(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) = 0;

    virtual void value_double(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void value_longlong(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void value_ulonglong(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void value_bool(bool value, const basic_parsing_context<Char>& context) = 0;
};

template <class Char>
class basic_null_json_input_handler : public basic_json_input_handler<Char>
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
protected:
    virtual void value_string(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void value_double(double value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void value_longlong(long long value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void value_ulonglong(unsigned long long value, const basic_parsing_context<Char>& context)
    {
    }

    virtual void value_bool(bool value, const basic_parsing_context<Char>& context)
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<char> null_json_input_handler;

}

#endif
