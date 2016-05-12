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

template <typename CharT>
class basic_json_input_output_adapter : public basic_json_input_handler<CharT>
{
public:
    basic_json_input_output_adapter()
        : writer_(std::addressof(null_json_output_handler<CharT>()))
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

    void do_begin_object(const basic_parsing_context<CharT>& context) override
    {
        writer_->begin_object();
    }

    void do_end_object(const basic_parsing_context<CharT>& context) override
    {
        writer_->end_object();
    }

    void do_begin_array(const basic_parsing_context<CharT>& context) override
    {
        writer_->begin_array();
    }

    void do_end_array(const basic_parsing_context<CharT>& context) override
    {
        writer_->end_array();
    }

    void do_name(const CharT* name, size_t length, 
                 const basic_parsing_context<CharT>& context) override
    {
        writer_->name(name, length);
    }

    void do_string_value(const CharT* value, size_t length, 
                         const basic_parsing_context<CharT>& context) override
    {
        writer_->value(value, length);
    }

    void do_integer_value(int64_t value, const basic_parsing_context<CharT>& context) override
    {
        writer_->value(value);
    }

    void do_uinteger_value(uint64_t value, 
                                 const basic_parsing_context<CharT>& context) override
    {
        writer_->value(value);
    }

    void do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context) override
    {
        writer_->value(value, precision);
    }

    void do_bool_value(bool value, const basic_parsing_context<CharT>& context) override
    {
        writer_->value(value);
    }

    void do_null_value(const basic_parsing_context<CharT>& context) override
    {
        writer_->value(null_type());
    }

    basic_json_output_handler<CharT>* writer_;
};

template <typename CharT>
class basic_json_filter : public basic_json_input_handler<CharT>
{
public:
    basic_json_filter(basic_json_input_handler<CharT>& handler)
        : handler_(std::addressof(handler)),
          err_handler_(std::addressof(basic_default_parse_error_handler<CharT>::instance()))
    {
    }

    basic_json_filter(basic_json_input_handler<CharT>& handler,
                      basic_parse_error_handler<CharT>& err_handler)
        : handler_(std::addressof(handler)),
          err_handler_(std::addressof(err_handler))
    {
    }

    basic_json_filter(basic_json_output_handler<CharT>& output_handler)
        : input_output_adapter_(output_handler), handler_(std::addressof(input_output_adapter_)),
          err_handler_(std::addressof(basic_default_parse_error_handler<CharT>::instance()))
    {
    }

    basic_json_filter(basic_json_output_handler<CharT>& output_handler,
                      basic_parse_error_handler<CharT>& err_handler)
        : input_output_adapter_(output_handler), handler_(std::addressof(input_output_adapter_)),
          err_handler_(std::addressof(err_handler))
    {
    }

    basic_json_input_handler<CharT>& input_handler()
    {
        return *handler_;
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_json_input_handler<CharT>& parent()
    {
        return *handler_;
    }
#endif

private:
    void do_begin_json() override
    {
        handler_->begin_json();
    }

    void do_end_json() override
    {
        handler_->end_json();
    }

    void do_begin_object(const basic_parsing_context<CharT>& context) override
    {
        handler_->begin_object(context);
    }

    void do_end_object(const basic_parsing_context<CharT>& context) override
    {
        handler_->end_object(context);
    }

    void do_begin_array(const basic_parsing_context<CharT>& context) override
    {
        handler_->begin_array(context);
    }

    void do_end_array(const basic_parsing_context<CharT>& context) override
    {
        handler_->end_array(context);
    }

    void do_name(const CharT* name, size_t length, const basic_parsing_context<CharT>& context) override
    {
        handler_->name(name, length, context);
    }

    void do_string_value(const CharT* value, size_t length, const basic_parsing_context<CharT>& context) override
    {
        handler_->value(value,length,context);
    }

    void do_double_value(double value, uint8_t precision, const basic_parsing_context<CharT>& context) override
    {
        handler_->value(value,precision,context);
    }

    void do_integer_value(int64_t value, const basic_parsing_context<CharT>& context) override
    {
        handler_->value(value,context);
    }

    void do_uinteger_value(uint64_t value, const basic_parsing_context<CharT>& context) override
    {
        handler_->value(value,context);
    }

    void do_bool_value(bool value, const basic_parsing_context<CharT>& context) override
    {
        handler_->value(value,context);
    }

    void do_null_value(const basic_parsing_context<CharT>& context) override
    {
        handler_->value(null_type(),context);
    }

    basic_json_input_output_adapter<CharT> input_output_adapter_;
    basic_json_input_handler<CharT>* handler_;
    basic_parse_error_handler<CharT>* err_handler_;
};

// Filters out begin_json and end_json events
template <typename CharT>
class basic_begin_end_json_filter : public basic_json_filter<CharT>
{
public:
    basic_begin_end_json_filter(basic_json_input_handler<CharT>& handler)
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

template <typename CharT>
class basic_json_output_input_adapter : public basic_json_output_handler<CharT>
{
public:
    basic_json_output_input_adapter(basic_json_input_handler<CharT>& input_handler,
                                    const basic_parsing_context<CharT>& context)
        : input_handler_(std::addressof(input_handler)),
          context_(std::addressof(context))
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
        input_handler_->begin_object(*context_);
    }

    void do_end_object() override
    {
        input_handler_->end_object(*context_);
    }

    void do_begin_array() override
    {
        input_handler_->begin_array(*context_);
    }

    void do_end_array() override
    {
        input_handler_->end_array(*context_);
    }

    void do_name(const CharT* name, size_t length) override
    {
        input_handler_->name(name, length, *context_);
    }

    void do_string_value(const CharT* value, size_t length) override
    {
        input_handler_->value(value, length, *context_);
    }

    void do_integer_value(int64_t value) override
    {
        input_handler_->value(value, *context_);
    }

    void do_uinteger_value(uint64_t value) override
    {
        input_handler_->value(value, *context_);
    }

    void do_double_value(double value, uint8_t precision) override
    {
        input_handler_->value(value, precision, *context_);
    }

    void do_bool_value(bool value) override
    {
        input_handler_->value(value, *context_);
    }

    void do_null_value() override
    {
        input_handler_->value(null_type(), *context_);
    }

    basic_json_input_handler<CharT>* input_handler_;
    const basic_parsing_context<CharT>* context_;
};

typedef basic_json_filter<char> json_filter;
typedef basic_json_filter<wchar_t> wjson_filter;

}

#endif
