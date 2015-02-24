// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_OUTPUT_HANDLER_HPP
#define JSONCONS_JSON_OUTPUT_HANDLER_HPP

#include <string>

namespace jsoncons {

template <typename Char>
class basic_json_output_handler
{
public:
    virtual ~basic_json_output_handler() {}

    // Overloaded methods

    void begin_json()
    {
        do_begin_json();
    }

    void end_json()
    {
        do_end_json();
    }

    void begin_object()
    {
        do_begin_object();
    }

    void end_object()
    {
        do_end_object();
    }

    void begin_array()
    {
        do_begin_array();
    }

    void end_array()
    {
        do_end_array();
    }

    void name(const std::basic_string<Char>& name)
    {
        do_name(name.c_str(), name.length());
    }

    void name(const Char* p, size_t length) 
    {
        do_name(p, length);
    }

    void value(const std::basic_string<Char>& value) 
    {
        do_string_value(value.c_str(), value.length());
    }

    void value(const Char* p, size_t length) 
    {
        do_string_value(p, length);
    }

    void value(const Char* p) 
    {
        do_string_value(p, std::char_traits<Char>::length(p));
    }

    void value(int value) 
    {
        do_longlong_value(value);
    }

    void value(long value) 
    {
        do_longlong_value(value);
    }

    void value(long long value) 
    {
        do_longlong_value(value);
    }

    void value(unsigned int value) 
    {
        do_ulonglong_value(value);
    }

    void value(unsigned long value) 
    {
        do_ulonglong_value(value);
    }

    void value(unsigned long long value) 
    {
        do_ulonglong_value(value);
    }

    void value(double value)
    {
        do_double_value(value);
    }

    void value(bool value) 
    {
        do_bool_value(value);
    }

    void value(null_type)
    {
        do_null_value();
    }

private:

    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_name(const Char* name, size_t length) = 0;

    virtual void do_begin_object() = 0;

    virtual void do_end_object() = 0;

    virtual void do_begin_array() = 0;

    virtual void do_end_array() = 0;

    virtual void do_null_value() = 0;

    virtual void do_string_value(const Char* value, size_t length) = 0;

    virtual void do_double_value(double value) = 0;

    virtual void do_longlong_value(long long value) = 0;

    virtual void do_ulonglong_value(unsigned long long value) = 0;

    virtual void do_bool_value(bool value) = 0;
};

template <typename Char>
class null_json_output_handler_impl : public basic_json_output_handler<Char>
{
private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_name(const Char* name, size_t length) override
    {
    }

    void do_begin_object() override
    {
    }

    void do_end_object() override
    {
    }

    void do_begin_array() override
    {
    }

    void do_end_array() override
    {
    }

    void do_null_value() override
    {
    }

    void do_string_value(const Char*, size_t length) override
    {
    }

    void do_double_value(double) override
    {
    }

    void do_longlong_value(long long) override
    {
    }

    void do_ulonglong_value(unsigned long long) override
    {
    }

    void do_bool_value(bool) override
    {
    }

};

template<typename Char>
basic_json_output_handler<Char>& null_json_output_handler()
{
    static null_json_output_handler_impl<Char> instance;
    return instance;
}

typedef basic_json_output_handler<char> json_output_handler;
typedef basic_json_output_handler<wchar_t> wjson_output_handler;

}
#endif
