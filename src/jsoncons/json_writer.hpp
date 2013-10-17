// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_OUT_STREAM_HPP
#define JSONCONS_JSON_OUT_STREAM_HPP

#include <string>

namespace jsoncons {

template <class Char>
class basic_json_writer
{
public:

    virtual void name(const std::basic_string<Char>& name) = 0;

    virtual void value(const std::basic_string<Char>& value) = 0;

    virtual void value(double value) = 0;

    virtual void value(long long value) = 0;

    virtual void value(unsigned long long value) = 0;

    virtual void value(bool value) = 0;

    virtual void null_value() = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;
};

template <class Char>
class basic_null_json_writer : public basic_json_writer<Char>
{
public:

    virtual void name(const std::basic_string<Char>& name) 
    {
    }

    virtual void value(const std::basic_string<Char>& value) 
    {
    }

    virtual void value(double value) 
    {
    }

    virtual void value(long long value) 
    {
    }

    virtual void value(unsigned long long value) 
    {
    }

    virtual void value(bool value) 
    {
    }

    virtual void null_value() 
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
};

typedef basic_null_json_writer<char> null_json_writer;

typedef basic_json_writer<char> json_writer;

}
#endif
