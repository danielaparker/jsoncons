// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_SERIALIZER_HPP
#define JSONCONS_JSON_SERIALIZER_HPP

#include <string>

namespace jsoncons {

template <class Char>
class base_data_box;

template <class Char>
class basic_json_serializer
{
public:

    virtual void begin_member(const std::basic_string<Char>& name) = 0;

    virtual void end_member() = 0;

    virtual void begin_element() = 0;

    virtual void end_element() = 0;

    virtual void value(const std::basic_string<Char>& value) = 0;

    virtual void userdata(const base_data_box<Char>& value) = 0;

    virtual void value(double value) = 0;

    virtual void value(long long value) = 0;

    virtual void value(unsigned long long value) = 0;

    virtual void value(bool value) = 0;

    virtual void null() = 0;

    virtual void begin_object() = 0;

    virtual void end_object() = 0;

    virtual void begin_array() = 0;

    virtual void end_array() = 0;
};

typedef basic_json_serializer<char> json_serializer;

}
#endif
