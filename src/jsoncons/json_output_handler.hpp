// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_OUTPUT_HANDLER_HPP
#define JSONCONS_JSON_OUTPUT_HANDLER_HPP

#include <string>
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template<typename CharT> 
void print_integer(int64_t value, buffered_ostream<CharT>& os)
{
    CharT buf[255];
    uint64_t u = (value < 0) ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    CharT* p = buf;
    do
    {
        *p++ = static_cast<CharT>(48 + u%10);
    }
    while (u /= 10);
    if (value < 0)
    {
        os.put('-');
    }
    while (--p >= buf)
    {
        os.put(*p);
    }
}

template<typename CharT>
void print_uinteger(uint64_t value, buffered_ostream<CharT>& os)
{
    CharT buf[255];
    CharT* p = buf;
    do
    {
        *p++ = static_cast<CharT>(48 + value % 10);
    } while (value /= 10);
    while (--p >= buf)
    {
        os.put(*p);
    }
}

template <typename CharT>
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

    void name(const std::basic_string<CharT>& name)
    {
        do_name(name.data(), name.length());
    }

    void name(const CharT* p, size_t length) 
    {
        do_name(p, length);
    }

    void value(const std::basic_string<CharT>& value) 
    {
        do_string_value(value.data(), value.length());
    }

    void value(const CharT* p, size_t length) 
    {
        do_string_value(p, length);
    }

    void value(const CharT* p) 
    {
        do_string_value(p, std::char_traits<CharT>::length(p));
    }

    void value(int value) 
    {
        do_integer_value(value);
    }

    void value(long value) 
    {
        do_integer_value(value);
    }

    void value(long long value) 
    {
        do_integer_value(value);
    }

    void value(unsigned int value) 
    {
        do_uinteger_value(value);
    }

    void value(unsigned long value) 
    {
        do_uinteger_value(value);
    }

    void value(unsigned long long value) 
    {
        do_uinteger_value(value);
    }

    void value(double value, uint8_t precision = 0)
    {
        do_double_value(value, precision);
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

    virtual void do_name(const CharT* name, size_t length) = 0;

    virtual void do_begin_object() = 0;

    virtual void do_end_object() = 0;

    virtual void do_begin_array() = 0;

    virtual void do_end_array() = 0;

    virtual void do_null_value() = 0;

    virtual void do_string_value(const CharT* value, size_t length) = 0;

    virtual void do_double_value(double value, uint8_t precision) = 0;

    virtual void do_integer_value(int64_t value) = 0;

    virtual void do_uinteger_value(uint64_t value) = 0;

    virtual void do_bool_value(bool value) = 0;
};

template <typename CharT>
class null_json_output_handler_impl : public basic_json_output_handler<CharT>
{
private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_name(const CharT* name, size_t length) override
    {
        (void)name;
        (void)length;
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

    void do_string_value(const CharT* p, size_t length) override
    {
        (void)p;
        (void)length;
    }

    void do_double_value(double, uint8_t) override
    {
    }

    void do_integer_value(int64_t) override
    {
    }

    void do_uinteger_value(uint64_t) override
    {
    }

    void do_bool_value(bool) override
    {
    }

};

template<typename CharT>
basic_json_output_handler<CharT>& null_json_output_handler()
{
    static null_json_output_handler_impl<CharT> instance;
    return instance;
}

typedef basic_json_output_handler<char> json_output_handler;
typedef basic_json_output_handler<wchar_t> wjson_output_handler;

}
#endif
