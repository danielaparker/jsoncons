// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_PARSER_HPP
#define JSONCONS_CSV_CSV_PARSER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <cctype>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons/json_parser.hpp"
#include "jsoncons/json_filter.hpp"
#include "jsoncons_ext/csv/csv_error_category.hpp"
#include "jsoncons_ext/csv/csv_parameters.hpp"

namespace jsoncons { namespace csv {

template <typename CharT>
struct json_csv_parser_traits
{
};

template <>
struct json_csv_parser_traits<char>
{
    static const std::string string_literal() {return "string";};

    static const std::string integer_literal() {return "integer";};

    static const std::string float_literal() {return "float";};

    static const std::string boolean_literal() {return "boolean";};
};

template <>
struct json_csv_parser_traits<wchar_t> // assume utf16
{
    static const std::wstring string_literal() {return L"string";};

    static const std::wstring integer_literal() {return L"integer";};

    static const std::wstring float_literal() {return L"float";};

    static const std::wstring boolean_literal() {return L"boolean";};
};

enum class csv_modes {
    done,
    header,
    array,
    object
};

enum class csv_states 
{
    start, 
    comment,
    expect_value,
    between_fields,
    quoted_string,
    unquoted_string,
    escaped_value,
    minus, 
    zero,  
    integer,
    fraction,
    exp1,
    exp2,
    exp3,
    done
};

enum class data_types
{
    string_t,integer_t,float_t,boolean_t
};

template<typename CharT>
class basic_csv_parser : private basic_parsing_context<CharT>
{
    static const int default_depth = 3;

    csv_states state_;
    int top_;
    std::vector<csv_modes> stack_;
    basic_json_input_handler<CharT> *handler_;
    basic_parse_error_handler<CharT> *err_handler_;
    bool is_negative_;
    uint32_t cp_;
    size_t index_;
    unsigned long column_;
    unsigned long line_;
    int curr_char_;
    int prev_char_;
    std::basic_string<CharT> string_buffer_;
    csv_states saved_state_;
    int depth_;
    basic_csv_parameters<CharT> parameters_;
    std::vector<std::basic_string<CharT>> column_names_;
    std::vector<data_types> column_types_;
    std::vector<std::basic_string<CharT>> column_defaults_;
    size_t column_index_;
    basic_begin_end_json_filter<CharT> filter_;
    basic_json_parser<CharT> parser_;

public:
    basic_csv_parser(basic_json_input_handler<CharT>& handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(basic_default_parse_error_handler<CharT>::instance())),
         is_negative_(false),
         cp_(0),
         index_(0),
         filter_(handler),
         parser_(filter_)
    {
        depth_ = default_depth;
        state_ = csv_states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     basic_csv_parameters<CharT> params)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(basic_default_parse_error_handler<CharT>::instance())),
         is_negative_(false),
         cp_(0),
         index_(0),
         parameters_(params),
         filter_(handler),
         parser_(filter_)
   {
        depth_ = default_depth;
        state_ = csv_states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     basic_parse_error_handler<CharT>& err_handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         is_negative_(false),
         cp_(0),
         index_(0),
         filter_(handler),
         parser_(filter_)
    {
        depth_ = default_depth;
        state_ = csv_states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     basic_parse_error_handler<CharT>& err_handler,
                     basic_csv_parameters<CharT> params)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         is_negative_(false),
         cp_(0),
         index_(0),
         parameters_(params),
         filter_(handler),
         parser_(filter_)
    {
        depth_ = default_depth;
        state_ = csv_states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    ~basic_csv_parser()
    {
    }

    const basic_parsing_context<CharT>& parsing_context() const
    {
        return *this;
    }

    bool done() const
    {
        return state_ == csv_states::done;
    }

    const std::vector<std::basic_string<CharT>>& column_labels() const
    {
        return column_names_;
    }

    void after_field()
    {
        ++column_index_;
    }

    void before_record()
    {
        if (column_index_ == 0)
        {
            switch (stack_[top_])
            {
            case csv_modes::array:
                handler_->begin_array(*this);
                break;
            case csv_modes::object:
                handler_->begin_object(*this);
                break;
            default:
                break;
            }
        }
    }

    void after_record()
    {
        switch (stack_[top_])
        {
        case csv_modes::array:
            handler_->end_array(*this);
            break;
        case csv_modes::object:
            handler_->end_object(*this);
            break;
        case csv_modes::header:
            if (line_ >= parameters_.header_lines())
            {
                if (column_names_.size() > 0)
                {
                    flip(csv_modes::header, csv_modes::object);
                }
                else
                {
                    flip(csv_modes::header, csv_modes::array);
                }
            }
            break;
        default:
            break;
        }
        column_index_ = 0;
    }

    void begin_parse()
    {
        push(csv_modes::done);
        handler_->begin_json();

        if (parameters_.column_names().size() > 0)
        {
            column_names_ = parameters_.column_names();
        }
#if !defined(JSONCONS_NO_DEPRECATED)
        else if (parameters_.header().length() > 0)
        {
            basic_empty_json_input_handler<CharT> ih;
            basic_csv_parameters<CharT> params;
            params.field_delimiter(parameters_.field_delimiter());
            params.quote_char(parameters_.quote_char());
            params.quote_escape_char(parameters_.quote_escape_char());
            params.assume_header(true);
            basic_csv_parser<CharT> p(ih,params);
            p.begin_parse();
            p.parse(parameters_.header().data(),0,parameters_.header().length());
            p.end_parse();
            column_names_ = p.column_labels();
        }
#endif
        if (parameters_.column_types().size() > 0)
        {
            column_types_.resize(parameters_.column_types().size());
            for (size_t i = 0; i < parameters_.column_types().size(); ++i)
            {
                if (parameters_.column_types()[i] == json_csv_parser_traits<CharT>::string_literal())
                {
                    column_types_[i] = data_types::string_t;
                }
                else if (parameters_.column_types()[i] == json_csv_parser_traits<CharT>::integer_literal())
                {
                    column_types_[i] = data_types::integer_t;
                }
                else if (parameters_.column_types()[i] == json_csv_parser_traits<CharT>::float_literal())
                {
                    column_types_[i] = data_types::float_t;
                }
                else if (parameters_.column_types()[i] == json_csv_parser_traits<CharT>::boolean_literal())
                {
                    column_types_[i] = data_types::boolean_t;
                }
            }
        }
#if !defined(JSONCONS_NO_DEPRECATED)
        else if (parameters_.data_types().length() > 0)
        {
            basic_empty_json_input_handler<CharT> ih;
            basic_csv_parameters<CharT> params;
            params.field_delimiter(parameters_.field_delimiter());
            params.assume_header(true);
            basic_csv_parser<CharT> p(ih,params);
            p.begin_parse();
            p.parse(parameters_.data_types().data(),0,parameters_.data_types().length());
            p.end_parse();
            column_types_.resize(p.column_labels().size());
            for (size_t i = 0; i < p.column_labels().size(); ++i)
            {
                if (p.column_labels()[i] == json_csv_parser_traits<CharT>::string_literal())
                {
                    column_types_[i] = data_types::string_t;
                }
                else if (p.column_labels()[i] == json_csv_parser_traits<CharT>::integer_literal())
                {
                    column_types_[i] = data_types::integer_t;
                }
                else if (p.column_labels()[i] == json_csv_parser_traits<CharT>::float_literal())
                {
                    column_types_[i] = data_types::float_t;
                }
                else if (p.column_labels()[i] == json_csv_parser_traits<CharT>::boolean_literal())
                {
                    column_types_[i] = data_types::boolean_t;
                }
            }
        }
#endif
        if (parameters_.column_defaults().size() > 0)
        {
            column_defaults_ = parameters_.column_defaults();
        }
#if !defined(JSONCONS_NO_DEPRECATED)
        else if (parameters_.default_values().length() > 0)
        {
            basic_empty_json_input_handler<CharT> ih;
            basic_csv_parameters<CharT> params;
            params.field_delimiter(parameters_.field_delimiter());
            params.assume_header(true);
            basic_csv_parser<CharT> p(ih,params);
            p.begin_parse();
            p.parse(parameters_.default_values().data(),0,parameters_.default_values().length());
            p.end_parse();
            column_defaults_.resize(p.column_labels().size());
            for (size_t i = 0; i < p.column_labels().size(); ++i)
            {
                column_defaults_[i] = p.column_labels()[i];
            }
        }
#endif
        if (parameters_.header_lines() > 0)
        {
            push(csv_modes::header);
        }
        else
        {
            push(csv_modes::array);
        }
        handler_->begin_array(*this);
        state_ = csv_states::expect_value;
        column_index_ = 0;
        prev_char_ = 0;
        curr_char_ = 0;
        column_ = 1;
    }

    void parse(const CharT* p, size_t start, size_t length)
    {
        index_ = start;
        for (; index_ < length && state_ != csv_states::done; ++index_)
        {
            curr_char_ = p[index_];
all_csv_states:
            switch (state_)
            {
            case csv_states::comment:
                if (curr_char_ == '\n')
                {
                    state_ = csv_states::expect_value;
                }
                else if (prev_char_ == '\r')
                {
                    state_ = csv_states::expect_value;
                    goto all_csv_states;
                }
                break;
            case csv_states::expect_value:
                if (column_ == 1 && curr_char_ == parameters_.comment_starter())
                {
                    state_ = csv_states::comment;
                }
                else
                {
                    state_ = csv_states::unquoted_string;
                    goto all_csv_states;
                }
                break;
            case csv_states::between_fields:
                if (curr_char_ == '\r' || (prev_char_ != '\r' && curr_char_ == '\n'))
                {
                    after_record();
                    state_ = csv_states::expect_value;
                }
                else if (curr_char_ == parameters_.field_delimiter())
                {
                    state_ = csv_states::expect_value;
                }
                break;
            case csv_states::escaped_value: 
                {
                    if (curr_char_ == parameters_.quote_char())
                    {
                        string_buffer_.push_back(curr_char_);
                        state_ = csv_states::quoted_string;
                    }
                    else if (parameters_.quote_escape_char() == parameters_.quote_char())
                    {
                        before_record();
                        end_quoted_string_value();
                        after_field();
                        state_ = csv_states::between_fields;
                        goto all_csv_states;
                    }
                }
                break;
            case csv_states::quoted_string: 
                {
                    if (curr_char_ == parameters_.quote_escape_char())
                    {
                        state_ = csv_states::escaped_value;
                    }
                    else if (curr_char_ == parameters_.quote_char())
                    {
                        before_record();
                        end_quoted_string_value();
                        after_field();
                        state_ = csv_states::between_fields;
                    }
                    else
                    {
                        string_buffer_.push_back(curr_char_);
                    }
                }
                break;
            case csv_states::unquoted_string: 
                {
                    if (curr_char_ == '\r' || (prev_char_ != '\r' && curr_char_ == '\n'))
                    {
                        before_record();
                        end_unquoted_string_value();
                        after_field();
                        after_record();
                        state_ = csv_states::expect_value;
                    }
                    else if (curr_char_ == '\n')
                    {
                        if (prev_char_ != '\r')
                        {
                            before_record();
                            end_unquoted_string_value();
                            after_field();
                            after_record();
                            state_ = csv_states::expect_value;
                        }
                    }
                    else if (curr_char_ == parameters_.field_delimiter())
                    {
                        before_record();
                        end_unquoted_string_value();
                        after_field();
                        state_ = csv_states::expect_value;
                    }
                    else if (curr_char_ == parameters_.quote_char())
                    {
                        string_buffer_.clear();
                        state_ = csv_states::quoted_string;
                    }
                    else
                    {
                        string_buffer_.push_back(curr_char_);
                    }
                }
                break;
            default:
                err_handler_->error(std::error_code(csv_parser_errc::invalid_state, csv_error_category()), *this);
                break;
            }
            if (line_ > parameters_.max_lines())
            {
                state_ = csv_states::done;
            }
            switch (curr_char_)
            {
            case '\r':
                ++line_;
                column_ = 1;
                break;
            case '\n':
                if (prev_char_ != '\r')
                {
                    ++line_;
                }
                column_ = 1;
                break;
            default:
                ++column_;
                break;
            }
            prev_char_ = curr_char_;
        }
    }

    void end_parse()
    {
        switch (state_)
        {
        case csv_states::unquoted_string: 
            before_record();
            end_unquoted_string_value();
            after_field();
            break;
        case csv_states::escaped_value:
            if (parameters_.quote_escape_char() == parameters_.quote_char())
            {
                before_record();
                end_quoted_string_value();
                after_field();
            }
            break;
        default:
            break;
        }
        if (column_index_ > 0)
        {
            after_record();
        }
        switch (stack_[top_])
        {
        case csv_modes::array:
            if (!pop(csv_modes::array))
            {
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_error_category()), *this);
            }
            break;
        case csv_modes::object:
            if (!pop(csv_modes::object))
            {
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_error_category()), *this);
            }
            break;
        case csv_modes::header:
            if (!pop(csv_modes::header))
            {
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_error_category()), *this);
            }
            break;
        default:
            break;
        }
        handler_->end_array(*this);
        if (!pop(csv_modes::done))
        {
            err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_error_category()), *this);
        }
        handler_->end_json();
    }

    csv_states state() const
    {
        return state_;
    }

    size_t index() const
    {
        return index_;
    }
private:

    void trim_string_buffer(bool trim_leading, bool trim_trailing)
    {
        size_t start = 0;
        size_t length = string_buffer_.length();
        if (trim_leading)
        {
            bool done = false;
            while (!done && start < string_buffer_.length())
            {
                if ((string_buffer_[start] < 256) && std::isspace(string_buffer_[start]))
                {
                    ++start;
                }
                else
                {
                    done = true;
                }
            }
        }
        if (trim_trailing)
        {
            bool done = false;
            while (!done && length > 0)
            {
                if ((string_buffer_[length-1] < 256) && std::isspace(string_buffer_[length-1]))
                {
                    --length;
                }
                else
                {
                    done = true;
                }
            }
        }
        if (start != 0 || length != string_buffer_.size())
        {
            string_buffer_ = string_buffer_.substr(start,length-start);
        }
    }

    void end_unquoted_string_value() 
    {
        if (parameters_.trim_leading() | parameters_.trim_trailing())
        {
            trim_string_buffer(parameters_.trim_leading(),parameters_.trim_trailing());
        }
        switch (stack_[top_])
        {
        case csv_modes::header:
            if (parameters_.assume_header() && line_ == 1)
            {
                column_names_.push_back(string_buffer_);
            }
            break;
        case csv_modes::object:
            if (!(parameters_.ignore_empty_values() && string_buffer_.size() == 0))
            {
                if (column_index_ < column_names_.size())
                {
                    handler_->name(column_names_[column_index_].data(), column_names_[column_index_].length(), *this);
                    if (parameters_.unquoted_empty_value_is_null() && string_buffer_.length() == 0)
                    {
                        handler_->value(jsoncons::null_type(),*this);
                    }
                    else
                    {
                        end_value();
                    }
                }
            }
            break;
        case csv_modes::array:
            if (parameters_.unquoted_empty_value_is_null() && string_buffer_.length() == 0)
            {
                handler_->value(jsoncons::null_type(),*this);
            }
            else
            {
                end_value();
            }
            break;
        default:
            err_handler_->error(std::error_code(csv_parser_errc::invalid_csv_text, csv_error_category()), *this);
            break;
        }
        state_ = csv_states::expect_value;
        string_buffer_.clear();
    }

    void end_quoted_string_value() 
    {
        if (parameters_.trim_leading_inside_quotes() | parameters_.trim_trailing_inside_quotes())
        {
            trim_string_buffer(parameters_.trim_leading_inside_quotes(),parameters_.trim_trailing_inside_quotes());
        }
        switch (stack_[top_])
        {
        case csv_modes::header:
            if (parameters_.assume_header() && line_ == 1)
            {
                column_names_.push_back(string_buffer_);
            }
            break;
        case csv_modes::object:
            if (!(parameters_.ignore_empty_values() && string_buffer_.size() == 0))
            {
                if (column_index_ < column_names_.size())
                {
                    handler_->name(column_names_[column_index_].data(), column_names_[column_index_].length(), *this);
                    end_value();
                }
            }
            break;
        case csv_modes::array:
            end_value();
            break;
        default:
            err_handler_->error(std::error_code(csv_parser_errc::invalid_csv_text, csv_error_category()), *this);
            break;
        }
        state_ = csv_states::expect_value;
        string_buffer_.clear();
    }

    void end_value()
    {
        if (column_index_ < column_types_.size())
        {
            switch (column_types_[column_index_])
            {
            case data_types::integer_t:
                {
                    std::istringstream iss(string_buffer_);
                    long long val;
                    iss >> val;
                    if (!iss.fail())
                    {
                        handler_->value(val, *this);
                    }
                    else
                    {
                        if (column_index_ < column_defaults_.size() && column_defaults_[column_index_].length() > 0)
                        {
                            parser_.begin_parse();
                            parser_.parse(column_defaults_[column_index_].data(),0,column_defaults_[column_index_].length());
                            parser_.end_parse();
                        }
                        else
                        {
                            handler_->value(null_type(), *this);
                        }
                    }
                }
                break;
            case data_types::float_t:
                {
                    std::istringstream iss(string_buffer_);
                    double val;
                    iss >> val;
                    if (!iss.fail())
                    {
                        handler_->value(val, 0, *this);
                    }
                    else
                    {
                        if (column_index_ < column_defaults_.size() && column_defaults_[column_index_].length() > 0)
                        {
                            parser_.begin_parse();
                            parser_.parse(column_defaults_[column_index_].data(),0,column_defaults_[column_index_].length());
                            parser_.end_parse();
                        }
                        else
                        {
                            handler_->value(null_type(), *this);
                        }
                    }
                }
                break;
            case data_types::boolean_t:
                {
                    if (string_buffer_.length() == 1 && string_buffer_[0] == '0')
                    {
                        handler_->value(false, *this);
                    }
                    else if (string_buffer_.length() == 1 && string_buffer_[0] == '1')
                    {
                        handler_->value(true, *this);
                    }
                    else if (string_buffer_.length() == 5 && ((string_buffer_[0] == 'f' || string_buffer_[0] == 'F') && (string_buffer_[1] == 'a' || string_buffer_[1] == 'A') && (string_buffer_[2] == 'l' || string_buffer_[2] == 'L') && (string_buffer_[3] == 's' || string_buffer_[3] == 'S') && (string_buffer_[4] == 'e' || string_buffer_[4] == 'E')))
                    {
                        handler_->value(false, *this);
                    }
                    else if (string_buffer_.length() == 4 && ((string_buffer_[0] == 't' || string_buffer_[0] == 'T') && (string_buffer_[1] == 'r' || string_buffer_[1] == 'R') && (string_buffer_[2] == 'u' || string_buffer_[2] == 'U') && (string_buffer_[3] == 'e' || string_buffer_[3] == 'E')))
                    {
                        handler_->value(true, *this);
                    }
                    else
                    {
                        if (column_index_ < column_defaults_.size() && column_defaults_[column_index_].length() > 0)
                        {
                            parser_.begin_parse();
                            parser_.parse(column_defaults_[column_index_].data(),0,column_defaults_[column_index_].length());
                            parser_.end_parse();
                        }
                        else
                        {
                            handler_->value(null_type(), *this);
                        }
                    }
                }
                break;
            default:
                if (string_buffer_.length() > 0)
                {
                    handler_->value(string_buffer_.data(), string_buffer_.length(), *this);
                }
                else
                {
                    if (column_index_ < column_defaults_.size() && column_defaults_[column_index_].length() > 0)
                    {
                        parser_.begin_parse();
                        parser_.parse(column_defaults_[column_index_].data(),0,column_defaults_[column_index_].length());
                        parser_.end_parse();
                    }
                    else
                    {
                        handler_->value("", *this);
                    }
                }
                break;  
            }
        }
        else
        {
            handler_->value(string_buffer_.data(), string_buffer_.length(), *this);
        }
    }

    size_t do_line_number() const override
    {
        return line_;
    }

    size_t do_column_number() const override
    {
        return column_;
    }

    CharT do_current_char() const override
    {
        return (CharT)prev_char_;
    }

    void push(csv_modes mode)
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_] = mode;
    }

    int peek()
    {
        return stack_[top_];
    }

    bool peek(csv_modes mode)
    {
        return stack_[top_] == mode;
    }

    bool flip(csv_modes mode1, csv_modes mode2)
    {
        if (top_ < 0 || stack_[top_] != mode1)
        {
            return false;
        }
        stack_[top_] = mode2;
        return true;
    }

    bool pop(csv_modes mode)
    {
        if (top_ < 0 || stack_[top_] != mode)
        {
            return false;
        }
        --top_;
        return true;
    }
};

typedef basic_csv_parser<char> csv_parser;
typedef basic_csv_parser<wchar_t> wcsv_parser;

}}

#endif

