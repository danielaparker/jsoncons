// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_FILTER_HPP
#define JSONCONS_JSON_FILTER_HPP

#include <string>

#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_output_handler.hpp"

namespace jsoncons {

template <typename Char>
class basic_json_input_output_adapter : public basic_json_input_handler<Char>
{
public:
    static null_basic_json_output_handler<Char> null_writer;

    basic_json_input_output_adapter()
        : writer_(std::addressof(null_writer))
    {
    }

    basic_json_input_output_adapter(basic_json_output_handler<Char>& handler)
        : writer_(std::addressof(handler))
    {
    }

    virtual void do_begin_json()
    {
        writer_->do_begin_json();
    }

    virtual void do_end_json()
    {
        writer_->do_end_json();
    }

    virtual void do_begin_object(const basic_parsing_context<Char>& context)
    {
        writer_->do_begin_object();
    }

    virtual void do_end_object(const basic_parsing_context<Char>& context)
    {
        writer_->do_end_object();
    }

    virtual void do_begin_array(const basic_parsing_context<Char>& context)
    {
        writer_->do_begin_array();
    }

    virtual void do_end_array(const basic_parsing_context<Char>& context)
    {
        writer_->do_end_array();
    }

    virtual void do_name(const Char* name, size_t length, 
                            const basic_parsing_context<Char>& context)
    {
        writer_->do_name(name, length);
    }

    virtual void do_null_value(const basic_parsing_context<Char>& context)
    {
        writer_->do_null_value();
    }

    virtual void do_string_value(const Char* value, size_t length, 
                              const basic_parsing_context<Char>& context)
    {
        writer_->do_string_value(value, length);
    }

    virtual void do_double_value(double value, const basic_parsing_context<Char>& context)
    {
        writer_->do_double_value(value);
    }

    virtual void do_longlong_value(long long value, const basic_parsing_context<Char>& context)
    {
        writer_->do_longlong_value(value);
    }

    virtual void do_ulonglong_value(unsigned long long value, 
                                 const basic_parsing_context<Char>& context)
    {
        writer_->do_ulonglong_value(value);
    }

    virtual void do_bool_value(bool value, const basic_parsing_context<Char>& context)
    {
        writer_->do_bool_value(value);
    }
private:
    basic_json_output_handler<Char>* writer_;
};

template <typename Char>
null_basic_json_output_handler<Char> basic_json_input_output_adapter<Char>::null_writer;

template <typename Char>
class basic_json_filter : public basic_json_input_handler<Char>
{
public:
    basic_json_filter(basic_json_input_handler<Char>& parent)
        : parent_(std::addressof(parent))
    {
    }

    basic_json_filter(basic_json_output_handler<Char>& output_handler)
        : input_output_adapter_(output_handler), parent_(std::addressof(input_output_adapter_))
    {
    }

    virtual void do_begin_json()
    {
        parent_->do_begin_json();
    }

    virtual void do_end_json()
    {
        parent_->do_end_json();
    }

    virtual void do_begin_object(const basic_parsing_context<Char>& context)
    {
        parent_->do_begin_object(context);
    }

    virtual void do_end_object(const basic_parsing_context<Char>& context)
    {
        parent_->do_end_object(context);
    }

    virtual void do_begin_array(const basic_parsing_context<Char>& context)
    {
        parent_->do_begin_array(context);
    }

    virtual void do_end_array(const basic_parsing_context<Char>& context)
    {
        parent_->do_end_array(context);
    }

    virtual void do_name(const Char* name, size_t length, const basic_parsing_context<Char>& context)
    {
        parent_->do_name(name, length, context);
    }

    virtual void do_null_value(const basic_parsing_context<Char>& context)
    {
        parent_->do_null_value(context);
    }

    basic_json_input_handler<Char>& parent()
    {
        return *parent_;
    }

// value(...) implementation
    virtual void do_string_value(const Char* value, size_t length, const basic_parsing_context<Char>& context)
    {
        parent_->value(value,length,context);
    }

    virtual void do_double_value(double value, const basic_parsing_context<Char>& context)
    {
        parent_->value(value,context);
    }

    virtual void do_longlong_value(long long value, const basic_parsing_context<Char>& context)
    {
        parent_->value(value,context);
    }

    virtual void do_ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
        parent_->value(value,context);
    }

    virtual void do_bool_value(bool value, const basic_parsing_context<Char>& context)
    {
        parent_->value(value,context);
    }
private:
    basic_json_input_output_adapter<Char> input_output_adapter_;
    basic_json_input_handler<Char>* parent_;
};

typedef basic_json_filter<char> json_filter;
typedef basic_json_filter<wchar_t> wjson_filter;

}

#endif
