// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template<typename CharT>
uint64_t string_to_uinteger(const CharT *s, size_t length) throw(std::overflow_error)
{
    static const uint64_t max_value = std::numeric_limits<uint64_t>::max JSONCONS_NO_MACRO_EXP();
    static const uint64_t max_value_div_10 = max_value / 10;
    uint64_t n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        uint64_t x = s[i] - '0';
        if (n > max_value_div_10)
        {
            throw std::overflow_error("Unsigned overflow");
        }
        n = n * 10;
        if (n > max_value - x)
        {
            throw std::overflow_error("Unsigned overflow");
        }

        n += x;
    }
    return n;
}

template<typename CharT>
int64_t string_to_integer(bool has_neg, const CharT *s, size_t length) throw(std::overflow_error)
{
    const long long max_value = std::numeric_limits<int64_t>::max JSONCONS_NO_MACRO_EXP();
    const long long max_value_div_10 = max_value / 10;

    long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        long long x = s[i] - '0';
        if (n > max_value_div_10)
        {
            throw std::overflow_error("Integer overflow");
        }
        n = n * 10;
        if (n > max_value - x)
        {
            throw std::overflow_error("Integer overflow");
        }

        n += x;
    }
    return has_neg ? -n : n;
}

template <typename CharT>
class basic_parsing_context;

template <typename CharT>
class basic_json_input_handler
{
public:
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

    void name(const std::basic_string<CharT>& name, const basic_parsing_context<CharT>& context)
    {
        do_name(name.data(), name.length(), context);
    }

    void name(const CharT* p, size_t length, const basic_parsing_context<CharT>& context) 
    {
        do_name(p, length, context);
    }

    void value(const std::basic_string<CharT>& value, const basic_parsing_context<CharT>& context) 
    {
        do_string_value(value.data(), value.length(), context);
    }

    void value(const CharT* p, size_t length, const basic_parsing_context<CharT>& context) 
    {
        do_string_value(p, length, context);
    }

    void value(const CharT* p, const basic_parsing_context<CharT>& context) 
    {
        do_string_value(p, std::char_traits<CharT>::length(p), context);
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

    virtual void do_name(const CharT* name, size_t length, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_null_value(const basic_parsing_context<CharT>& context) = 0;

    virtual void do_string_value(const CharT* value, size_t length, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_integer_value(int64_t value, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_uinteger_value(uint64_t value, const basic_parsing_context<CharT>& context) = 0;

    virtual void do_bool_value(bool value, const basic_parsing_context<CharT>& context) = 0;
};


template <typename CharT>
class basic_empty_json_input_handler : public basic_json_input_handler<CharT>
{
public:
    static basic_json_input_handler<CharT>& instance()
    {
        static basic_empty_json_input_handler<CharT> instance;
        return instance;
    }
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

    void do_name(const CharT* p, size_t length, const basic_parsing_context<CharT>&) override
    {
        (void)p;
        (void)length;
    }

    void do_null_value(const basic_parsing_context<CharT>&) override
    {
    }

    void do_string_value(const CharT* p, size_t length, const basic_parsing_context<CharT>&) override
    {
        (void)p;
        (void)length;
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

typedef basic_empty_json_input_handler<char> empty_json_input_handler;
typedef basic_empty_json_input_handler<wchar_t> wempty_json_input_handler;

}

#endif
