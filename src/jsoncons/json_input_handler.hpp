// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_INPUT_HANDLER_HPP
#define JSONCONS_JSON_INPUT_HANDLER_HPP

#include <string>
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template <typename Char>
class basic_parsing_context;

template <typename Char>
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

    void begin_object(const basic_parsing_context<Char>& context)
    {
        do_begin_object(context);
    }

    void end_object(const basic_parsing_context<Char>& context)
    {
        do_end_object(context);
    }

    void begin_array(const basic_parsing_context<Char>& context)
    {
        do_begin_array(context);
    }

    void end_array(const basic_parsing_context<Char>& context)
    {
        do_end_array(context);
    }

    void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        do_name(name.c_str(), name.length(), context);
    }

    void name(const Char* p, size_t length, const basic_parsing_context<Char>& context) 
    {
        do_name(p, length, context);
    }

    void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context) 
    {
        do_string_value(value.c_str(), value.length(), context);
    }

    void value(const Char* p, size_t length, const basic_parsing_context<Char>& context) 
    {
        do_string_value(p, length, context);
    }

    void value(const Char* p, const basic_parsing_context<Char>& context) 
    {
        do_string_value(p, std::char_traits<Char>::length(p), context);
    }

    void value(int value, const basic_parsing_context<Char>& context) 
    {
        do_longlong_value(value,context);
    }

    void value(long value, const basic_parsing_context<Char>& context) 
    {
        do_longlong_value(value,context);
    }

    void value(long long value, const basic_parsing_context<Char>& context) 
    {
        do_longlong_value(value,context);
    }

    void value(unsigned int value, const basic_parsing_context<Char>& context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(unsigned long value, const basic_parsing_context<Char>& context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(unsigned long long value, const basic_parsing_context<Char>& context) 
    {
        do_ulonglong_value(value,context);
    }

    void value(float value, const basic_parsing_context<Char>& context)
    {
        do_double_value(value, context);
    }

    void value(double value, const basic_parsing_context<Char>& context)
    {
        do_double_value(value, context);
    }

    void value(bool value, const basic_parsing_context<Char>& context) 
    {
        do_bool_value(value,context);
    }

    void value(null_type, const basic_parsing_context<Char>& context)
    {
        do_null_value(context);
    }

private:
    virtual void do_begin_json() = 0;

    virtual void do_end_json() = 0;

    virtual void do_begin_object(const basic_parsing_context<Char>& context) = 0;

    virtual void do_end_object(const basic_parsing_context<Char>& context) = 0;

    virtual void do_begin_array(const basic_parsing_context<Char>& context) = 0;

    virtual void do_end_array(const basic_parsing_context<Char>& context) = 0;

    virtual void do_name(const Char* name, size_t length, const basic_parsing_context<Char>& context) = 0;

    virtual void do_null_value(const basic_parsing_context<Char>& context) = 0;

    virtual void do_string_value(const Char* value, size_t length, const basic_parsing_context<Char>& context) = 0;

    virtual void do_double_value(double value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_longlong_value(long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context) = 0;

    virtual void do_bool_value(bool value, const basic_parsing_context<Char>& context) = 0;
};


template <typename Char>
class empty_basic_json_input_handler : public basic_json_input_handler<Char>
{
public:
    static basic_json_input_handler<Char>& instance()
    {
        static empty_basic_json_input_handler<Char> instance;
        return instance;
    }
private:
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_begin_object(const basic_parsing_context<Char>&) override
    {
    }

    void do_end_object(const basic_parsing_context<Char>&) override
    {
    }

    void do_begin_array(const basic_parsing_context<Char>&) override
    {
    }

    void do_end_array(const basic_parsing_context<Char>&) override
    {
    }

    void do_name(const Char* p, size_t length, const basic_parsing_context<Char>&) override
    {
    }

    void do_null_value(const basic_parsing_context<Char>&) override
    {
    }

    void do_string_value(const Char*, size_t length, const basic_parsing_context<Char>&) override
    {
    }

    void do_double_value(double, const basic_parsing_context<Char>&) override
    {
    }

    void do_longlong_value(long long, const basic_parsing_context<Char>&) override
    {
    }

    void do_ulonglong_value(unsigned long long, const basic_parsing_context<Char>&) override
    {
    }

    void do_bool_value(bool, const basic_parsing_context<Char>&) override
    {
    }
};

typedef basic_json_input_handler<char> json_input_handler;
typedef basic_json_input_handler<wchar_t> wjson_input_handler;

typedef empty_basic_json_input_handler<char> empty_json_input_handler;
typedef empty_basic_json_input_handler<wchar_t> wempty_json_input_handler;

}

#endif
