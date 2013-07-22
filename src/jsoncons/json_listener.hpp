// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_IN_STREAM_HPP
#define JSONCONS_JSON_IN_STREAM_HPP

#include <string>

namespace jsoncons {

template <class Char>
class basic_parsing_context;

template <class Char>
class basic_json_listener
{
public:
    virtual void begin_json() = 0;

    virtual void end_json() = 0;

    virtual void begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context) = 0;

    virtual void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) = 0;

    virtual void value(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void value(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void value(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void value(bool value, const basic_parsing_context<Char>& context) = 0;

    virtual void null_value(const basic_parsing_context<Char>& context) = 0;
};

typedef basic_json_listener<char> json_listener;

}

#endif
