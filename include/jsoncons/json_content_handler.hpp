// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONTENT_HANDLER_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/serializing_context.hpp>
#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons/json_type_traits.hpp> // for null_type
#endif

namespace jsoncons {

template <class CharT>
class basic_json_content_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view_ext<char_type,char_traits_type> string_view_type;

    virtual ~basic_json_content_handler() {}

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
        do_begin_object(null_serializing_context());
    }

    void begin_object(const serializing_context& context)
    {
        do_begin_object(context);
    }

    void begin_object(size_t length)
    {
        do_begin_object(length, null_serializing_context());
    }

    void begin_object(size_t length, const serializing_context& context)
    {
        do_begin_object(length, context);
    }

    void end_object()
    {
        do_end_object(null_serializing_context());
    }

    void end_object(const serializing_context& context)
    {
        do_end_object(context);
    }

    void begin_array()
    {
        do_begin_array(null_serializing_context());
    }

    void begin_array(size_t length)
    {
        do_begin_array(length, null_serializing_context());
    }

    void begin_array(const serializing_context& context)
    {
        do_begin_array(context);
    }

    void begin_array(size_t length, const serializing_context& context)
    {
        do_begin_array(length, context);
    }

    void end_array()
    {
        do_end_array(null_serializing_context());
    }

    void end_array(const serializing_context& context)
    {
        do_end_array(context);
    }

    void name(const string_view_type& name)
    {
        do_name(name, null_serializing_context());
    }

    void name(const string_view_type& name, const serializing_context& context)
    {
        do_name(name, context);
    }

    void string_value(const string_view_type& value) 
    {
        do_string_value(value, null_serializing_context());
    }

    void string_value(const string_view_type& value, const serializing_context& context) 
    {
        do_string_value(value, context);
    }

    void byte_string_value(const uint8_t* data, size_t length) 
    {
        do_byte_string_value(data, length, null_serializing_context());
    }

    void byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) 
    {
        do_byte_string_value(data, length, context);
    }

    void integer_value(int64_t value)
    {
        do_integer_value(value,null_serializing_context());
    }

    void integer_value(int64_t value, const serializing_context& context)
    {
        do_integer_value(value,context);
    }

    void uinteger_value(uint64_t value)
    {
        do_uinteger_value(value,null_serializing_context());
    }

    void uinteger_value(uint64_t value, const serializing_context& context)
    {
        do_uinteger_value(value,context);
    }

    void double_value(double value)
    {
        do_double_value(value, number_format(), null_serializing_context());
    }

    void double_value(double value, uint8_t precision)
    {
        do_double_value(value, number_format(precision, 0), null_serializing_context());
    }

    void double_value(double value, const number_format& fmt)
    {
        do_double_value(value, fmt, null_serializing_context());
    }

    void double_value(double value, const serializing_context& context)
    {
        do_double_value(value, number_format(), context);
    }

    void double_value(double value, uint8_t precision, const serializing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void double_value(double value, const number_format& fmt, const serializing_context& context)
    {
        do_double_value(value, fmt, context);
    }

    void bool_value(bool value) 
    {
        do_bool_value(value,null_serializing_context());
    }

    void bool_value(bool value, const serializing_context& context) 
    {
        do_bool_value(value,context);
    }

    void null_value() 
    {
        do_null_value(null_serializing_context());
    }

    void null_value(const serializing_context& context) 
    {
        do_null_value(context);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void name(const CharT* p, size_t length, const serializing_context& context) 
    {
        do_name(string_view_type(p, length), context);
    }

    void value(const std::basic_string<CharT>& value, const serializing_context& context) 
    {
        do_string_value(value, context);
    }

    void value(const CharT* p, size_t length, const serializing_context& context) 
    {
        do_string_value(string_view_type(p, length), context);
    }

    void value(const CharT* p, const serializing_context& context)
    {
        do_string_value(string_view_type(p), context);
    }

    void value(int value, const serializing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(long value, const serializing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(long long value, const serializing_context& context) 
    {
        do_integer_value(value,context);
    }

    void value(unsigned int value, const serializing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long value, const serializing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long long value, const serializing_context& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(float value, uint8_t precision, const serializing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void value(double value, uint8_t precision, const serializing_context& context)
    {
        do_double_value(value, number_format(precision, 0), context);
    }

    void value(bool value, const serializing_context& context) 
    {
        do_bool_value(value,context);
    }

    void value(null_type, const serializing_context& context)
    {
        do_null_value(context);
    }
#endif

private:
    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_begin_object(const serializing_context& context) = 0;

    virtual void do_begin_object(size_t length, const serializing_context& context) 
    {
        do_begin_object(context);
    }

    virtual void do_end_object(const serializing_context& context) = 0;

    virtual void do_begin_array(const serializing_context& context) = 0;

    virtual void do_begin_array(size_t length, const serializing_context& context) 
    {
        do_begin_array(context);
    }

    virtual void do_end_array(const serializing_context& context) = 0;

    virtual void do_name(const string_view_type& name, const serializing_context& context) = 0;

    virtual void do_null_value(const serializing_context& context) = 0;

    virtual void do_string_value(const string_view_type& value, const serializing_context& context) = 0;

    virtual void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) = 0;

    virtual void do_double_value(double value, const number_format& fmt, const serializing_context& context) = 0;

    virtual void do_integer_value(int64_t value, const serializing_context& context) = 0;

    virtual void do_uinteger_value(uint64_t value, const serializing_context& context) = 0;

    virtual void do_bool_value(bool value, const serializing_context& context) = 0;
};

template <class CharT>
class basic_null_json_content_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type                                 ;
private:
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_begin_object(const serializing_context&) override
    {
    }

    void do_end_object(const serializing_context&) override
    {
    }

    void do_begin_array(const serializing_context&) override
    {
    }

    void do_end_array(const serializing_context&) override
    {
    }

    void do_name(const string_view_type&, const serializing_context&) override
    {
    }

    void do_null_value(const serializing_context&) override
    {
    }

    void do_string_value(const string_view_type&, const serializing_context&) override
    {
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context&) override
    {
    }

    void do_double_value(double, const number_format& fmt, const serializing_context&) override
    {
    }

    void do_integer_value(int64_t, const serializing_context&) override
    {
    }

    void do_uinteger_value(uint64_t, const serializing_context&) override
    {
    }

    void do_bool_value(bool, const serializing_context&) override
    {
    }
};

typedef basic_json_content_handler<char> json_content_handler;
typedef basic_json_content_handler<wchar_t> wjson_content_handler;

typedef basic_null_json_content_handler<char> null_json_content_handler;
typedef basic_null_json_content_handler<wchar_t> wnull_json_content_handler;

}

#endif
