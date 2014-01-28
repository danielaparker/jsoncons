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

template <class C>
class basic_parsing_context;

template <class C>
class basic_json_input_handler
{
public:
    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<C>& context) = 0;

    virtual void end_object(const basic_parsing_context<C>& context) = 0;

    virtual void begin_array(const basic_parsing_context<C>& context) = 0;

    virtual void end_array(const basic_parsing_context<C>& context) = 0;

    virtual void name(const std::basic_string<C>& name, const basic_parsing_context<C>& context) = 0;

    virtual void value(const std::basic_string<C>& value, const basic_parsing_context<C>& context) = 0;

    virtual void value(double value, const basic_parsing_context<C>& context) = 0;

    virtual void value(long long value, const basic_parsing_context<C>& context) = 0;

    virtual void value(unsigned long long value, const basic_parsing_context<C>& context) = 0;

    virtual void value(bool value, const basic_parsing_context<C>& context) = 0;

    virtual void null_value(const basic_parsing_context<C>& context) = 0;
};

template <class C>
class basic_null_json_input_handler : public basic_json_input_handler<C>
{
public:
    virtual void begin_json() 
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<C>& context) 
    {
    }

    virtual void end_object(const basic_parsing_context<C>& context) 
    {
    }

    virtual void begin_array(const basic_parsing_context<C>& context) 
    {
    }

    virtual void end_array(const basic_parsing_context<C>& context) 
    {
    }

    virtual void name(const std::basic_string<C>& name, const basic_parsing_context<C>& context) 
    {
    }

    virtual void value(const std::basic_string<C>& value, const basic_parsing_context<C>& context) 
    {
    }

    virtual void value(double value, const basic_parsing_context<C>& context) 
    {
    }

    virtual void value(long long value, const basic_parsing_context<C>& context) 
    {
    }

    virtual void value(unsigned long long value, const basic_parsing_context<C>& context) 
    {
    }

    virtual void value(bool value, const basic_parsing_context<C>& context) 
    {
    }

    virtual void null_value(const basic_parsing_context<C>& context) 
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<char> null_json_input_handler;

}

#endif
