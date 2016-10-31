// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_FILTER_HPP
#define JSONCONS_JSON_FILTER_HPP

#include <string>

#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"

namespace jsoncons {

template <class CharT>
class basic_json_input_output_adapter : public basic_json_input_handler<CharT>
{
private:
    basic_null_json_output_handler<CharT> null_output_handler_;
    basic_json_output_handler<CharT>* writer_;
public:
    basic_json_input_output_adapter()
        : writer_(&null_output_handler_)
    {
    }

    basic_json_input_output_adapter(basic_json_output_handler<CharT>& handler)
        : writer_(std::addressof(handler))
    {
    }

private:

    void do_begin_json() override
    {
        writer_->begin_json();
    }

    void do_end_json() override
    {
        writer_->end_json();
    }

    void do_begin_object(const basic_parsing_context<CharT>&) override
    {
        writer_->begin_object();
    }

    void do_end_object(const basic_parsing_context<CharT>&) override
    {
        writer_->end_object();
    }

    void do_begin_array(const basic_parsing_context<CharT>&) override
    {
        writer_->begin_array();
    }

    void do_end_array(const basic_parsing_context<CharT>&) override
    {
        writer_->end_array();
    }

    void do_name(const CharT* name, size_t length, 
                 const basic_parsing_context<CharT>&) override
    {
        writer_->name(name, length);
    }

    void do_string_value(const CharT* value, size_t length, 
                         const basic_parsing_context<CharT>&) override
    {
        writer_->value(value, length);
    }

    void do_integer_value(int64_t value, const basic_parsing_context<CharT>&) override
    {
        writer_->value(value);
    }

    void do_uinteger_value(uint64_t value, 
                           const basic_parsing_context<CharT>&) override
    {
        writer_->value(value);
    }

    void do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>&) override
    {
        writer_->value(value, precision);
    }

    void do_bool_value(bool value, const basic_parsing_context<CharT>&) override
    {
        writer_->value(value);
    }

    void do_null_value(const basic_parsing_context<CharT>&) override
    {
        writer_->value(null_type());
    }
};

template <class CharT>
class basic_json_output_input_adapter : public basic_json_output_handler<CharT>
{
private:

    class null_parsing_context : public parsing_context
    {
        size_t do_line_number() const override {return 1;}

        size_t do_column_number() const override {return 1;}

        char do_current_char() const override {return '0';}
    };
    basic_null_json_input_handler<CharT> null_input_handler_;

    const null_parsing_context context_;
    basic_json_input_handler<CharT>* input_handler_;

public:
    basic_json_output_input_adapter()
        : input_handler_(&null_input_handler_)
    {
    }
    basic_json_output_input_adapter(basic_json_input_handler<CharT>& input_handler)
        : input_handler_(std::addressof(input_handler))
    {
    }

private:

    void do_begin_json() override
    {
        input_handler_->begin_json();
    }

    void do_end_json() override
    {
        input_handler_->end_json();
    }

    void do_begin_object() override
    {
        input_handler_->begin_object(context_);
    }

    void do_end_object() override
    {
        input_handler_->end_object(context_);
    }

    void do_begin_array() override
    {
        input_handler_->begin_array(context_);
    }

    void do_end_array() override
    {
        input_handler_->end_array(context_);
    }

    void do_name(const CharT* name, size_t length) override
    {
        input_handler_->name(name, length, context_);
    }

    void do_string_value(const CharT* value, size_t length) override
    {
        input_handler_->value(value, length, context_);
    }

    void do_integer_value(int64_t value) override
    {
        input_handler_->value(value, context_);
    }

    void do_uinteger_value(uint64_t value) override
    {
        input_handler_->value(value, context_);
    }

    void do_double_value(double value, uint8_t precision) override
    {
        input_handler_->value(value, precision, context_);
    }

    void do_bool_value(bool value) override
    {
        input_handler_->value(value, context_);
    }

    void do_null_value() override
    {
        input_handler_->value(null_type(), context_);
    }
};

template <class CharT>
class basic_json_filter : public basic_json_output_handler<CharT>
{
private:
    basic_json_input_output_adapter<CharT> input_output_adapter_;
    basic_json_output_input_adapter<CharT> output_input_adapter_;
    basic_json_output_handler<CharT>* output_handler_;
    basic_json_input_handler<CharT>* input_handler_;
public:
    basic_json_filter(basic_json_output_handler<CharT>& handler)
        : input_output_adapter_(*this),
          output_handler_(std::addressof(handler)),
          input_handler_(std::addressof(input_output_adapter_))
    {
    }

    basic_json_filter(basic_json_input_handler<CharT>& handler)
        : input_output_adapter_(*this),
          output_input_adapter_(handler),
          output_handler_(std::addressof(output_input_adapter_)),
          input_handler_(std::addressof(input_output_adapter_))
    {
    }

    operator basic_json_input_handler<CharT>&() 
    { 
        return *input_handler_; 
    }

    basic_json_output_handler<CharT>& parent_handler()
    {
        return *output_handler_;
    }

private:
    void do_begin_json() override
    {
        output_handler_->begin_json();
    }

    void do_end_json() override
    {
        output_handler_->end_json();
    }

    void do_begin_object() override
    {
        output_handler_->begin_object();
    }

    void do_end_object() override
    {
        output_handler_->end_object();
    }

    void do_begin_array() override
    {
        output_handler_->begin_array();
    }

    void do_end_array() override
    {
        output_handler_->end_array();
    }

    void do_name(const CharT* name, size_t length) override
    {
        output_handler_->name(name, length);
    }

    void do_string_value(const CharT* value, size_t length) override
    {
        output_handler_->value(value,length);
    }

    void do_double_value(double value, uint8_t precision) override
    {
        output_handler_->value(value,precision);
    }

    void do_integer_value(int64_t value) override
    {
        output_handler_->value(value);
    }

    void do_uinteger_value(uint64_t value) override
    {
        output_handler_->value(value);
    }

    void do_bool_value(bool value) override
    {
        output_handler_->value(value);
    }

    void do_null_value() override
    {
        output_handler_->value(null_type());
    }

};

// Filters out begin_json and end_json events
template <class CharT>
class basic_json_body_filter : public basic_json_filter<CharT>
{
public:
    basic_json_body_filter(basic_json_input_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler)
    {
    }
private:
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }
};

template <class CharT>
class basic_rename_name_filter : public basic_json_filter<CharT>
{
private:
    std::basic_string<CharT> name_;
    std::basic_string<CharT> new_name_;
public:
    basic_rename_name_filter(const std::basic_string<CharT>& name,
                             const std::basic_string<CharT>& new_name,
                             basic_json_output_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler), 
          name_(name), new_name_(new_name)
    {
    }

    basic_rename_name_filter(const std::basic_string<CharT>& name,
                             const std::basic_string<CharT>& new_name,
                             basic_json_input_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler), 
          name_(name), new_name_(new_name)
    {
    }

private:
    void do_name(const CharT* p, size_t length) override
    {
        size_t len = std::min JSONCONS_NO_MACRO_EXP(name_.length(),length);
        if (len == length && std::char_traits<CharT>::compare(name_.data(),p,len) == 0)
        {
            parent_handler().name(new_name_.data(),new_name_.length());
        }
        else
        {
            parent_handler().name(p,length);
        }
    }
};

typedef basic_json_filter<char> json_filter;
typedef basic_json_filter<wchar_t> wjson_filter;
typedef basic_rename_name_filter<char> rename_name_filter;
typedef basic_rename_name_filter<wchar_t> wrename_name_filter;

}

#endif
