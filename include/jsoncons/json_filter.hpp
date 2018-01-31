// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_FILTER_HPP
#define JSONCONS_JSON_FILTER_HPP

#include <string>

#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>

namespace jsoncons {

template <class CharT>
class basic_json_input_output_handler_adapter : public basic_json_input_handler<CharT>
{
public:
    using typename basic_json_input_handler<CharT>::string_view_type;
private:

    basic_null_json_output_handler<CharT> null_output_handler_;
    basic_json_output_handler<CharT>& output_handler_;

    // noncopyable and nonmoveable
    basic_json_input_output_handler_adapter<CharT>(const basic_json_input_output_handler_adapter<CharT>&) = delete;
    basic_json_input_output_handler_adapter<CharT>& operator=(const basic_json_input_output_handler_adapter<CharT>&) = delete;

public:
    basic_json_input_output_handler_adapter()
        : output_handler_(null_output_handler_)
    {
    }

    basic_json_input_output_handler_adapter(basic_json_output_handler<CharT>& handler)
        : output_handler_(handler)
    {
    }

private:

    void do_begin_json() override
    {
        output_handler_.begin_json();
    }

    void do_end_json() override
    {
        output_handler_.end_json();
    }

    void do_begin_object(const parsing_context&) override
    {
        output_handler_.begin_object();
    }

    void do_end_object(const parsing_context&) override
    {
        output_handler_.end_object();
    }

    void do_begin_array(const parsing_context&) override
    {
        output_handler_.begin_array();
    }

    void do_end_array(const parsing_context&) override
    {
        output_handler_.end_array();
    }

    void do_name(const string_view_type& name, 
                 const parsing_context&) override
    {
        output_handler_.name(name);
    }

    void do_string_value(const string_view_type& value, 
                         const parsing_context&) override
    {
        output_handler_.string_value(value);
    }

    void do_byte_string_value(const uint8_t* data, size_t length, 
                                const parsing_context&) override
    {
        output_handler_.byte_string_value(data, length);
    }

    void do_integer_value(int64_t value, const parsing_context&) override
    {
        output_handler_.integer_value(value);
    }

    void do_uinteger_value(uint64_t value, 
                           const parsing_context&) override
    {
        output_handler_.uinteger_value(value);
    }

    void do_double_value(double value, const number_format& fmt, const parsing_context&) override
    {
        output_handler_.double_value(value, fmt);
    }

    void do_bool_value(bool value, const parsing_context&) override
    {
        output_handler_.bool_value(value);
    }

    void do_null_value(const parsing_context&) override
    {
        output_handler_.null_value();
    }
};

template <class CharT>
class basic_json_output_input_handler_adapter : public basic_json_output_handler<CharT>
{
public:
    using typename basic_json_output_handler<CharT>::string_view_type                                 ;
private:
    class null_parsing_context : public parsing_context
    {
        size_t do_line_number() const override { return 0; }

        size_t do_column_number() const override { return 0; }
    };
    const null_parsing_context default_context_ = null_parsing_context();

    basic_null_json_input_handler<CharT> null_input_handler_;
    basic_json_input_output_handler_adapter<CharT> default_input_output_adapter_;
    basic_json_input_handler<CharT>& input_handler_;
    const basic_json_input_output_handler_adapter<CharT>& input_output_adapter_;

    // noncopyable and nonmoveable
    basic_json_output_input_handler_adapter<CharT>(const basic_json_output_input_handler_adapter<CharT>&) = delete;
    basic_json_output_input_handler_adapter<CharT>& operator=(const basic_json_output_input_handler_adapter<CharT>&) = delete;

public:
    basic_json_output_input_handler_adapter()
        : input_handler_(null_input_handler_),
          input_output_adapter_(default_input_output_adapter_)
    {
    }
    basic_json_output_input_handler_adapter(basic_json_input_handler<CharT>& input_handler)
        : input_handler_(input_handler),
          input_output_adapter_(default_input_output_adapter_)
    {
    }
    basic_json_output_input_handler_adapter(basic_json_input_handler<CharT>& input_handler,
                                    const basic_json_input_output_handler_adapter<CharT>& input_output_adapter)
        : input_handler_(input_handler),
          input_output_adapter_(input_output_adapter)
    {
    }

private:

    void do_begin_json() override
    {
        input_handler_.begin_json();
    }

    void do_end_json() override
    {
        input_handler_.end_json();
    }

    void do_begin_object() override
    {
        input_handler_.begin_object(default_context_);
    }

    void do_end_object() override
    {
        input_handler_.end_object(default_context_);
    }

    void do_begin_array() override
    {
        input_handler_.begin_array(default_context_);
    }

    void do_end_array() override
    {
        input_handler_.end_array(default_context_);
    }

    void do_name(const string_view_type& name) override
    {
        input_handler_.name(name, default_context_);
    }

    void do_string_value(const string_view_type& value) override
    {
        input_handler_.string_value(value, default_context_);
    }

    void do_byte_string_value(const uint8_t* data, size_t length) override
    {
        input_handler_.byte_string_value(data, length, default_context_);
    }

    void do_integer_value(int64_t value) override
    {
        input_handler_.integer_value(value, default_context_);
    }

    void do_uinteger_value(uint64_t value) override
    {
        input_handler_.uinteger_value(value, default_context_);
    }

    void do_double_value(double value, const number_format& fmt) override
    {
        input_handler_.double_value(value, fmt, default_context_);
    }

    void do_bool_value(bool value) override
    {
        input_handler_.bool_value(value, default_context_);
    }

    void do_null_value() override
    {
        input_handler_.null_value(default_context_);
    }
};

template <class CharT>
class basic_json_filter : public basic_json_input_handler<CharT>
{
public:
    using typename basic_json_input_handler<CharT>::string_view_type                                 ;
private:
    basic_json_input_output_handler_adapter<CharT> input_output_adapter_;
    basic_json_output_input_handler_adapter<CharT> output_input_adapter_;
    basic_json_output_handler<CharT>& output_handler_;
    basic_json_input_handler<CharT>& downstream_handler_;

    // noncopyable and nonmoveable
    basic_json_filter<CharT>(const basic_json_filter<CharT>&) = delete;
    basic_json_filter<CharT>& operator=(const basic_json_filter<CharT>&) = delete;
public:
    basic_json_filter(basic_json_output_handler<CharT>& handler)
        : input_output_adapter_(handler),
          output_input_adapter_(*this),
          output_handler_(output_input_adapter_),
          downstream_handler_(input_output_adapter_)
    {
    }

    basic_json_filter(basic_json_input_handler<CharT>& handler)
        : output_input_adapter_(*this),
          output_handler_(output_input_adapter_),
          downstream_handler_(handler)
    {
    }

    operator basic_json_output_handler<CharT>&() 
    { 
        return output_handler_; 
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_json_input_handler<CharT>& input_handler()
    {
        return downstream_handler_;
    }
#endif

    basic_json_input_handler<CharT>& downstream_handler()
    {
        return downstream_handler_;
    }

private:
    void do_begin_json() override
    {
        downstream_handler_.begin_json();
    }

    void do_end_json() override
    {
        downstream_handler_.end_json();
    }

    void do_begin_object(const parsing_context& context) override
    {
        downstream_handler_.begin_object(context);
    }

    void do_end_object(const parsing_context& context) override
    {
        downstream_handler_.end_object(context);
    }

    void do_begin_array(const parsing_context& context) override
    {
        downstream_handler_.begin_array(context);
    }

    void do_end_array(const parsing_context& context) override
    {
        downstream_handler_.end_array(context);
    }

    void do_name(const string_view_type& name,
                 const parsing_context& context) override
    {
        downstream_handler_.name(name,context);
    }

    void do_string_value(const string_view_type& value,
                         const parsing_context& context) override
    {
        downstream_handler_.string_value(value,context);
    }

    void do_byte_string_value(const uint8_t* data, size_t length,
                              const parsing_context& context) override
    {
        downstream_handler_.byte_string_value(data, length, context);
    }

    void do_double_value(double value, const number_format& fmt,
                 const parsing_context& context) override
    {
        downstream_handler_.double_value(value, fmt, context);
    }

    void do_integer_value(int64_t value,
                 const parsing_context& context) override
    {
        downstream_handler_.integer_value(value,context);
    }

    void do_uinteger_value(uint64_t value,
                 const parsing_context& context) override
    {
        downstream_handler_.uinteger_value(value,context);
    }

    void do_bool_value(bool value,
                 const parsing_context& context) override
    {
        downstream_handler_.bool_value(value,context);
    }

    void do_null_value(const parsing_context& context) override
    {
        downstream_handler_.null_value(context);
    }

};

// Filters out begin_json and end_json events
template <class CharT>
class basic_json_fragment_filter : public basic_json_filter<CharT>
{
public:
    using typename basic_json_filter<CharT>::string_view_type;

    basic_json_fragment_filter(basic_json_input_handler<CharT>& handler)
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
class basic_rename_object_member_filter : public basic_json_filter<CharT>
{
public:
    using typename basic_json_filter<CharT>::string_view_type;

private:
    std::basic_string<CharT> name_;
    std::basic_string<CharT> new_name_;
public:
    basic_rename_object_member_filter(const std::basic_string<CharT>& name,
                             const std::basic_string<CharT>& new_name,
                             basic_json_output_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler), 
          name_(name), new_name_(new_name)
    {
    }

    basic_rename_object_member_filter(const std::basic_string<CharT>& name,
                             const std::basic_string<CharT>& new_name,
                             basic_json_input_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler), 
          name_(name), new_name_(new_name)
    {
    }

private:
    void do_name(const string_view_type& name,
                 const parsing_context& context) override
    {
        if (name == name_)
        {
            this->downstream_handler().name(new_name_,context);
        }
        else
        {
            this->downstream_handler().name(name,context);
        }
    }
};

typedef basic_json_filter<char> json_filter;
typedef basic_json_filter<wchar_t> wjson_filter;
typedef basic_rename_object_member_filter<char> rename_object_member_filter;
typedef basic_rename_object_member_filter<wchar_t> wrename_object_member_filter;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_rename_object_member_filter<char> rename_name_filter;
typedef basic_rename_object_member_filter<wchar_t> wrename_name_filter;
#endif

}

#endif
