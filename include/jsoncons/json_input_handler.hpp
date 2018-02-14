// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons/json_type_traits.hpp> // for null_type
#endif

namespace jsoncons {

class parsing_context;

template <class CharT>
class basic_json_input_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view_ext<char_type,char_traits_type> string_view_type;

    virtual ~basic_json_input_handler() {}

    void begin_json()
    {
        do_begin_json();
    }

    void end_json()
    {
        do_end_json();
    }

    void begin_object(const parsing_context& context)
    {
        do_begin_object(context);
    }

    void end_object(const parsing_context& context)
    {
        do_end_object(context);
    }

    void begin_array(const parsing_context& context)
    {
        do_begin_array(context);
    }

    void end_array(const parsing_context& context)
    {
        do_end_array(context);
    }

    void name(const string_view_type& name, const parsing_context& context)
    {
        do_name(name, context);
    }

// new

    void string_value(const string_view_type& value, const parsing_context& context) 
    {
        do_string_value(value, context);
    }

    void byte_string_value(const uint8_t* data, size_t length, const parsing_context& context) 
    {
        do_byte_string_value(data, length, context);
    }

    void integer_value(int64_t value, const parsing_context& context)
    {
        do_integer_value(value,context);
    }

    void uinteger_value(uint64_t value, const parsing_context& context)
    {
        do_uinteger_value(value,context);
    }

    void double_value(double value, const parsing_context& context)
    {
        do_double_value(value, number_format(), context);
    }

    void double_value(double value, uint8_t precision, const parsing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void double_value(double value, const number_format& fmt, const parsing_context& context)
    {
        do_double_value(value, fmt, context);
    }

    void bool_value(bool value, const parsing_context& context) 
    {
        do_bool_value(value,context);
    }

    void null_value(const parsing_context& context) 
    {
        do_null_value(context);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void name(const CharT* p, size_t length, const parsing_context& context) 
    {
        do_name(string_view_type(p, length), context);
    }

    void value(const std::basic_string<CharT>& value, const parsing_context& context) 
    {
        do_string_value(value, context);
    }

    void value(const CharT* p, size_t length, const parsing_context& context) 
    {
        do_string_value(string_view_type(p, length), context);
    }

    void value(const CharT* p, const parsing_context& context)
    {
        do_string_value(string_view_type(p), context);
    }

    void value(int value, const parsing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(long value, const parsing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(long long value, const parsing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(unsigned int value, const parsing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long value, const parsing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long long value, const parsing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(float value, uint8_t precision, const parsing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void value(double value, uint8_t precision, const parsing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void value(bool value, const parsing_context& context) 
    {
        do_bool_value(value,context);
    }

    void value(null_type, const parsing_context& context)
    {
        do_null_value(context);
    }
#endif

private:
    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_begin_object(const parsing_context& context) = 0;

    virtual void do_end_object(const parsing_context& context) = 0;

    virtual void do_begin_array(const parsing_context& context) = 0;

    virtual void do_end_array(const parsing_context& context) = 0;

    virtual void do_name(const string_view_type& name, const parsing_context& context) = 0;

    virtual void do_null_value(const parsing_context& context) = 0;

    virtual void do_string_value(const string_view_type& value, const parsing_context& context) = 0;

    virtual void do_byte_string_value(const uint8_t* data, size_t length, const parsing_context& context) = 0;

    virtual void do_double_value(double value, const number_format& fmt, const parsing_context& context) = 0;

    virtual void do_integer_value(int64_t value, const parsing_context& context) = 0;

    virtual void do_uinteger_value(uint64_t value, const parsing_context& context) = 0;

    virtual void do_bool_value(bool value, const parsing_context& context) = 0;
};

template <class CharT>
class basic_null_json_input_handler final : public basic_json_input_handler<CharT>
{
public:
    using typename basic_json_input_handler<CharT>::string_view_type                                 ;
private:
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_begin_object(const parsing_context&) override
    {
    }

    void do_end_object(const parsing_context&) override
    {
    }

    void do_begin_array(const parsing_context&) override
    {
    }

    void do_end_array(const parsing_context&) override
    {
    }

    void do_name(const string_view_type&, const parsing_context&) override
    {
    }

    void do_null_value(const parsing_context&) override
    {
    }

    void do_string_value(const string_view_type&, const parsing_context&) override
    {
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const parsing_context&) override
    {
    }

    void do_double_value(double, const number_format& fmt, const parsing_context&) override
    {
    }

    void do_integer_value(int64_t, const parsing_context&) override
    {
    }

    void do_uinteger_value(uint64_t, const parsing_context&) override
    {
    }

    void do_bool_value(bool, const parsing_context&) override
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef basic_null_json_input_handler<char> null_json_input_handler;
typedef basic_null_json_input_handler<wchar_t> wnull_json_input_handler;

}

#endif
