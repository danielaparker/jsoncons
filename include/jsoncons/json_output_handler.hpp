// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_OUTPUT_HANDLER_HPP
#define JSONCONS_JSON_OUTPUT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/jsoncons_utilities.hpp>
#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons/json_type_traits.hpp> // for null_type
#endif

namespace jsoncons {

template<class CharT> 
void print_integer(int64_t value, buffered_output<CharT>& os)
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

template<class CharT>
void print_uinteger(uint64_t value, buffered_output<CharT>& os)
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

template <class CharT>
class basic_json_output_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<char_type,char_traits_type> string_view_type;
#else
    typedef std::basic_string_view<char_type,char_traits_type> string_view_type;
#endif

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

    void name(string_view_type name)
    {
        do_name(name);
    }

    void string_value(string_view_type value) 
    {
        do_string_value(value);
    }

    void integer_value(int64_t value) 
    {
        do_integer_value(value);
    }

    void uinteger_value(uint64_t value) 
    {
        do_uinteger_value(value);
    }

    void double_value(double value) 
    {
        do_double_value(value, std::numeric_limits<double>::digits10);
    }

    void double_value(double value, uint8_t precision) 
    {
        do_double_value(value, precision);
    }

    void bool_value(bool value) 
    {
        do_bool_value(value);
    }

    void null_value() 
    {
        do_null_value();
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void name(const CharT* p, size_t length) 
    {
        do_name(string_view_type(p, length));
    }

    void value(string_view_type value) 
    {
        do_string_value(value);
    }

    void value(const CharT* p, size_t length) 
    {
        do_string_value(string_view_type(p, length));
    }

    void value(const CharT* p) 
    {
        do_string_value(string_view_type(p));
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
#endif

private:

    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_name(string_view_type name) = 0;

    virtual void do_begin_object() = 0;

    virtual void do_end_object() = 0;

    virtual void do_begin_array() = 0;

    virtual void do_end_array() = 0;

    virtual void do_null_value() = 0;

    virtual void do_string_value(string_view_type value) = 0;

    virtual void do_double_value(double value, uint8_t precision) = 0;

    virtual void do_integer_value(int64_t value) = 0;

    virtual void do_uinteger_value(uint64_t value) = 0;

    virtual void do_bool_value(bool value) = 0;
};

template <class CharT>
class basic_null_json_output_handler : public basic_json_output_handler<CharT>
{
public:
    using typename basic_json_output_handler<CharT>::string_view_type                                 ;
private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_name(string_view_type) override
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

    void do_string_value(string_view_type) override
    {
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

typedef basic_json_output_handler<char> json_output_handler;
typedef basic_json_output_handler<wchar_t> wjson_output_handler;

}
#endif
