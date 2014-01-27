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

template <class C>
class basic_json_input_output_adapter : public basic_json_input_handler<C>
{
public:
    static basic_null_json_output_handler<C> null_writer;

    basic_json_input_output_adapter()
        : writer_(null_writer)
    {
    }

    basic_json_input_output_adapter(basic_json_output_handler<C>& handler)
        : writer_(handler)
    {
    }

    virtual void begin_json()
    {
        writer_.begin_json();
    }

    virtual void end_json()
    {
        writer_.end_json();
    }

    virtual void begin_object(const basic_parsing_context<C>& context)
    {
        writer_.begin_object();
    }

    virtual void end_object(const basic_parsing_context<C>& context)
    {
        writer_.end_object();
    }

    virtual void begin_array(const basic_parsing_context<C>& context)
    {
        writer_.begin_array();
    }

    virtual void end_array(const basic_parsing_context<C>& context)
    {
        writer_.end_array();
    }

    virtual void name(const std::basic_string<C>& name, const basic_parsing_context<C>& context)
    {
        writer_.name(name);
    }

    virtual void value(const std::basic_string<C>& value, const basic_parsing_context<C>& context)
    {
        writer_.value(value);
    }

    virtual void value(double value, const basic_parsing_context<C>& context)
    {
        writer_.value(value);
    }

    virtual void value(long long value, const basic_parsing_context<C>& context)
    {
        writer_.value(value);
    }

    virtual void value(unsigned long long value, const basic_parsing_context<C>& context)
    {
        writer_.value(value);
    }

    virtual void value(bool value, const basic_parsing_context<C>& context)
    {
        writer_.value(value);
    }

    virtual void null_value(const basic_parsing_context<C>& context)
    {
        writer_.null_value();
    }
private:
    basic_json_output_handler<C>& writer_;
};

template <class C>
basic_null_json_output_handler<C> basic_json_input_output_adapter<C>::null_writer;

template <class C>
class basic_json_filter : public basic_json_input_handler<C>
{
public:
    basic_json_filter(basic_json_input_handler<C>& parent)
        : parent_(parent)
    {
    }

    basic_json_filter(basic_json_output_handler<C>& output_handler)
        : input_output_adapter_(output_handler), parent_(input_output_adapter_)
    {
    }

    virtual void begin_json()
    {
        parent_.begin_json();
    }

    virtual void end_json()
    {
        parent_.end_json();
    }

    virtual void begin_object(const basic_parsing_context<C>& context)
    {
        parent_.begin_object(context);
    }

    virtual void end_object(const basic_parsing_context<C>& context)
    {
        parent_.end_object(context);
    }

    virtual void begin_array(const basic_parsing_context<C>& context)
    {
        parent_.begin_array(context);
    }

    virtual void end_array(const basic_parsing_context<C>& context)
    {
        parent_.end_array(context);
    }

    virtual void name(const std::basic_string<C>& name, const basic_parsing_context<C>& context)
    {
        parent_.name(name,context);
    }

    virtual void value(const std::basic_string<C>& value, const basic_parsing_context<C>& context)
    {
        parent_.value(value,context);
    }

    virtual void value(double value, const basic_parsing_context<C>& context)
    {
        parent_.value(value,context);
    }

    virtual void value(long long value, const basic_parsing_context<C>& context)
    {
        parent_.value(value,context);
    }

    virtual void value(unsigned long long value, const basic_parsing_context<C>& context)
    {
        parent_.value(value,context);
    }

    virtual void value(bool value, const basic_parsing_context<C>& context)
    {
        parent_.value(value,context);
    }

    virtual void null_value(const basic_parsing_context<C>& context)
    {
        parent_.null_value(context);
    }

    basic_json_input_handler<C>& parent() 
    {
        return parent_;
    }
private:
    basic_json_input_output_adapter<C> input_output_adapter_;
    basic_json_input_handler<C>& parent_;
};

typedef basic_json_filter<char> json_filter;

}

#endif
