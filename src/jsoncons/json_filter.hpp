// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_FILTER_HPP
#define JSONCONS_JSON_FILTER_HPP

#include <string>

#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_output_handler.hpp"

namespace jsoncons {

template <class Char>
class basic_json_input_output_adapter : public basic_json_input_handler<Char>
{
public:
    static basic_null_json_output_handler<Char> null_writer;

    basic_json_input_output_adapter()
        : writer_(null_writer)
    {
    }

    basic_json_input_output_adapter(basic_json_output_handler<Char>& handler)
        : writer_(handler)
    {
    }

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<Char>& context)
    {
        writer_.begin_object();
    }

    virtual void end_object(const basic_parsing_context<Char>& context)
    {
        writer_.end_object();
    }

    virtual void begin_array(const basic_parsing_context<Char>& context)
    {
        writer_.begin_array();
    }

    virtual void end_array(const basic_parsing_context<Char>& context)
    {
        writer_.end_array();
    }

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        writer_.name(name);
    }

    virtual void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
        writer_.value(value);
    }

    virtual void value(double value, const basic_parsing_context<Char>& context)
    {
        writer_.value(value);
    }

    virtual void value(long long value, const basic_parsing_context<Char>& context)
    {
        writer_.value(value);
    }

    virtual void value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
        writer_.value(value);
    }

    virtual void value(bool value, const basic_parsing_context<Char>& context)
    {
        writer_.value(value);
    }

    virtual void null_value(const basic_parsing_context<Char>& context)
    {
        writer_.null_value();
    }
private:
    basic_json_output_handler<Char>& writer_;
};

template <class Char>
basic_null_json_output_handler<Char> basic_json_input_output_adapter<Char>::null_writer;

template <class Char>
class basic_json_filter : public basic_json_input_handler<Char>
{
public:
    basic_json_filter(basic_json_input_handler<Char>& handler)
        : input_handler_(handler)
    {
    }

    basic_json_filter(basic_json_output_handler<Char>& handler)
        : listener_writer_(handler), input_handler_(listener_writer_)
    {
    }

    virtual void begin_json()
    {
        input_handler_.begin_json();
    }

    virtual void end_json()
    {
        input_handler_.end_json();
    }

    virtual void begin_object(const basic_parsing_context<Char>& context)
    {
        input_handler_.begin_object(context);
    }

    virtual void end_object(const basic_parsing_context<Char>& context)
    {
        input_handler_.end_object(context);
    }

    virtual void begin_array(const basic_parsing_context<Char>& context)
    {
        input_handler_.begin_array(context);
    }

    virtual void end_array(const basic_parsing_context<Char>& context)
    {
        input_handler_.end_array(context);
    }

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        input_handler_.name(name,context);
    }

    virtual void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
        input_handler_.value(value,context);
    }

    virtual void value(double value, const basic_parsing_context<Char>& context)
    {
        input_handler_.value(value,context);
    }

    virtual void value(long long value, const basic_parsing_context<Char>& context)
    {
        input_handler_.value(value,context);
    }

    virtual void value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
        input_handler_.value(value,context);
    }

    virtual void value(bool value, const basic_parsing_context<Char>& context)
    {
        input_handler_.value(value,context);
    }

    virtual void null_value(const basic_parsing_context<Char>& context)
    {
        input_handler_.null_value(context);
    }

    basic_json_input_handler<Char>& input_handler() 
    {
        return input_handler_;
    }
private:
    basic_json_input_output_adapter<Char> listener_writer_;
    basic_json_input_handler<Char>& input_handler_;
};

typedef basic_json_filter<char> json_filter;

}

#endif
