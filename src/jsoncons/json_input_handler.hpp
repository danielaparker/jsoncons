// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include <jsoncons/jsoncons.hpp>
#include <jsoncons/jsoncons_util.hpp>

namespace jsoncons {

template <class CharT>
class basic_parsing_context;

template <class CharT>
class basic_json_input_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<char_type,char_traits_type> string_view_type;
#else
    typedef std::basic_string_view<char_type,char_traits_type> string_view_type;
#endif

    virtual ~basic_json_input_handler() {}

    void begin_json()
    {
        do_begin_json();
    }

    void end_json()
    {
        do_end_json();
    }

    void begin_object(const basic_parsing_context<CharT>& context)
    {
        do_begin_object(context);
    }

    void end_object(const basic_parsing_context<CharT>& context)
    {
        do_end_object(context);
    }

    void begin_array(const basic_parsing_context<CharT>& context)
    {
        do_begin_array(context);
    }

    void end_array(const basic_parsing_context<CharT>& context)
    {
        do_end_array(context);
    }

    void name(string_view_type name, const basic_parsing_context<CharT>& context)
    {
        do_name(name, context);
    }

    void name(const CharT* p, size_t length, const basic_parsing_context<CharT>& context) 
    {
        do_name(string_view_type(p, length), context);
    }

    void value(const std::basic_string<CharT>& value, const basic_parsing_context<CharT>& context) 
    {
        do_string_value(value, context);
    }

    void value(const CharT* p, size_t length, const basic_parsing_context<CharT>& context) 
    {
        do_string_value(string_view_type(p, length), context);
    }

    void value(const CharT* p, const basic_parsing_context<CharT>& context)
    {
        do_string_value(string_view_type(p), context);
    }

    void value(int value, const basic_parsing_context<CharT>& context) 
    {
        do_integer_value(value,context);
    }

    void value(long value, const basic_parsing_context<CharT>& context) 
    {
        do_integer_value(value,context);
    }

    void value(long long value, const basic_parsing_context<CharT>& context) 
    {
        do_integer_value(value,context);
    }

    void value(unsigned int value, const basic_parsing_context<CharT>& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long value, const basic_parsing_context<CharT>& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<CharT>& context) 
    {
        do_uinteger_value(value,context);
    }

    void value(float value, uint8_t precision, const basic_parsing_context<CharT>& context)
    {
        do_double_value(value, precision, context);
    }

    void value(double value, uint8_t precision, const basic_parsing_context<CharT>& context)
    {
        do_double_value(value, precision, context);
    }

    void value(bool value, const basic_parsing_context<CharT>& context) 
    {
        do_bool_value(value,context);
    }

    void value(null_type, const basic_parsing_context<CharT>& context)
    {
        do_null_value(context);
    }

private:
    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_begin_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_end_object(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_begin_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_end_array(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_name(string_view_type name, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_null_value(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_string_value(string_view_type value, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_integer_value(int64_t value, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_uinteger_value(uint64_t value, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_bool_value(bool value, const basic_parsing_context<CharT>& context) = 0;
};

template <class CharT>
class basic_null_json_input_handler : public basic_json_input_handler<CharT>
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

    void do_begin_object(const basic_parsing_context<CharT>&) override
    {
    }

    void do_end_object(const basic_parsing_context<CharT>&) override
    {
    }

    void do_begin_array(const basic_parsing_context<CharT>&) override
    {
    }

    void do_end_array(const basic_parsing_context<CharT>&) override
    {
    }

    void do_name(string_view_type, const basic_parsing_context<CharT>&) override
    {
    }

    void do_null_value(const basic_parsing_context<CharT>&) override
    {
    }

    void do_string_value(string_view_type, const basic_parsing_context<CharT>&) override
    {
    }

    void do_double_value(double, uint8_t, const basic_parsing_context<CharT>&) override
    {
    }

    void do_integer_value(int64_t, const basic_parsing_context<CharT>&) override
    {
    }

    void do_uinteger_value(uint64_t, const basic_parsing_context<CharT>&) override
    {
    }

    void do_bool_value(bool, const basic_parsing_context<CharT>&) override
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef basic_null_json_input_handler<char> empty_json_input_handler;
typedef basic_null_json_input_handler<wchar_t> wempty_json_input_handler;

}

#endif
