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
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons_ext/csv/csv_error_category.hpp>
#include <jsoncons_ext/csv/csv_parameters.hpp>

namespace jsoncons { namespace csv {

enum class csv_mode_type 
{
    initial,
    header,
    data
};

enum class csv_state_type 
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

template<class CharT,class Allocator=std::allocator<CharT>>
class basic_csv_parser : private parsing_context
{
    typedef basic_string_view_ext<CharT> string_view_type;
    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type> string_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type> string_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<csv_mode_type> csv_mode_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<csv_type_info> csv_type_info_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::vector<string_type,string_allocator_type>> string_vector_allocator_type;

    static const int default_depth = 3;

    default_parse_error_handler default_err_handler_;
    csv_state_type state_;
    int top_;
    std::vector<csv_mode_type,csv_mode_allocator_type> stack_;
    basic_json_input_handler<CharT>& handler_;
    parse_error_handler& err_handler_;
    size_t index_;
    unsigned long column_;
    unsigned long line_;
    CharT curr_char_;
    CharT prev_char_;
    string_type value_buffer_;
    int depth_;
    basic_csv_parameters<CharT,Allocator> parameters_;
    std::vector<string_type,string_allocator_type> column_names_;
    std::vector<std::vector<string_type,string_allocator_type>,string_vector_allocator_type> column_values_;
    std::vector<csv_type_info,csv_type_info_allocator_type> column_types_;
    std::vector<string_type,string_allocator_type> column_defaults_;
    size_t column_index_;
    basic_json_fragment_filter<CharT> filter_;
    size_t level_;
    size_t offset_;

public:
    basic_csv_parser(basic_json_input_handler<CharT>& handler)
       : top_(-1),
         stack_(default_depth),
         handler_(handler),
         err_handler_(default_err_handler_),
         index_(0),
         filter_(handler),
         level_(0),
         offset_(0)
    {
        depth_ = default_depth;
        state_ = csv_state_type::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     basic_csv_parameters<CharT,Allocator> params)
       : top_(-1),
         stack_(default_depth),
         handler_(handler),
         err_handler_(default_err_handler_),
         index_(0),
         parameters_(params),
         filter_(handler),
         level_(0),
         offset_(0)
   {
        depth_ = default_depth;
        state_ = csv_state_type::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     parse_error_handler& err_handler)
       : top_(-1),
         stack_(default_depth),
         handler_(handler),
         err_handler_(err_handler),
         index_(0),
         filter_(handler),
         level_(0),
         offset_(0)
    {
        depth_ = default_depth;
        state_ = csv_state_type::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    basic_csv_parser(basic_json_input_handler<CharT>& handler,
                     parse_error_handler& err_handler,
                     basic_csv_parameters<CharT,Allocator> params)
       : top_(-1),
         stack_(default_depth),
         handler_(handler),
         err_handler_(err_handler),
         index_(0),
         parameters_(params),
         filter_(handler),
         level_(0),
         offset_(0)
    {
        depth_ = default_depth;
        state_ = csv_state_type::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
    }

    ~basic_csv_parser()
    {
    }

    const parsing_context& parsing_context() const
    {
        return *this;
    }

    bool done() const
    {
        return state_ == csv_state_type::done;
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
        offset_ = 0;
        if (stack_[top_] == csv_mode_type::data)
        {
            switch (parameters_.mapping())
            {
            case mapping_type::n_rows:
                handler_.begin_array(*this);
                break;
            case mapping_type::n_objects:
                handler_.begin_object(*this);
                break;
            case mapping_type::m_columns:
                break;
            default:
                break;
            }
        }
    }

    void after_record()
    {
        if (column_types_.size() > 0)
        {
            if (level_ > 0)
            {
                handler_.end_array(*this);
                level_ = 0;
            }
        }
        if (stack_[top_] == csv_mode_type::header)
        {
            if (line_ >= parameters_.header_lines())
            {
                flip(csv_mode_type::header, csv_mode_type::data);
            }
            column_values_.resize(column_names_.size());
            switch (parameters_.mapping())
            {
            case mapping_type::n_rows:
                if (column_names_.size() > 0)
                {
                    handler_.begin_array(*this);
                    for (const auto& name : column_names_)
                    {
                        end_value(name,column_index_);
                    }
                    handler_.end_array(*this);
                }
                break;
            default:
                break;
            }
        }
        else if (stack_[top_] == csv_mode_type::data)
        {
            switch (parameters_.mapping())
            {
            case mapping_type::n_rows:
                handler_.end_array(*this);
                break;
            case mapping_type::n_objects:
                handler_.end_object(*this);
                break;
            default:
                break;
            }
        }
        column_index_ = 0;
    }

    void reset()
    {
        push_mode(csv_mode_type::initial);
        handler_.begin_json();

        for (auto name : parameters_.column_names())
        {
            column_names_.emplace_back(name.data(),name.size());
        }
        for (auto name : parameters_.column_types())
        {
            column_types_.push_back(name);
        }
        for (auto name : parameters_.column_defaults())
        {
            column_defaults_.emplace_back(name.data(), name.size());
        }
        if (parameters_.header_lines() > 0)
        {
            push_mode(csv_mode_type::header);
        }
        else
        {
            push_mode(csv_mode_type::data);
        }
        if (parameters_.mapping() != mapping_type::m_columns)
        {
            handler_.begin_array(*this);
        }
        state_ = csv_state_type::expect_value;
        column_index_ = 0;
        prev_char_ = 0;
        curr_char_ = 0;
        column_ = 1;
        level_ = 0;
    }

    void parse(const CharT* p, size_t start, size_t length)
    {
        std::error_code ec;
        parse(p, start, length, ec);
        if (ec)
        {
            throw parse_error(ec,line_,column_);
        }
    }

    void parse(const CharT* p, size_t start, size_t length, std::error_code& ec)
    {
        index_ = start;
        for (; index_ < length && state_ != csv_state_type::done; ++index_)
        {
            curr_char_ = p[index_];
all_csv_states:
            switch (state_)
            {
            case csv_state_type::comment:
                if (curr_char_ == '\n')
                {
                    state_ = csv_state_type::expect_value;
                }
                else if (prev_char_ == '\r')
                {
                    state_ = csv_state_type::expect_value;
                    goto all_csv_states;
                }
                break;
            case csv_state_type::expect_value:
                if (column_ == 1 && curr_char_ == parameters_.comment_starter())
                {
                    state_ = csv_state_type::comment;
                }
                else
                {
                    state_ = csv_state_type::unquoted_string;
                    goto all_csv_states;
                }
                break;
            case csv_state_type::between_fields:
                if (curr_char_ == '\r' || (prev_char_ != '\r' && curr_char_ == '\n'))
                {
                    after_record();
                    state_ = csv_state_type::expect_value;
                }
                else if (curr_char_ == parameters_.field_delimiter())
                {
                    state_ = csv_state_type::expect_value;
                }
                break;
            case csv_state_type::escaped_value: 
                {
                    if (curr_char_ == parameters_.quote_char())
                    {
                        value_buffer_.push_back(static_cast<CharT>(curr_char_));
                        state_ = csv_state_type::quoted_string;
                    }
                    else if (parameters_.quote_escape_char() == parameters_.quote_char())
                    {
                        if (column_index_ == 0)
                        {
                            before_record();
                        }
                        end_quoted_string_value(ec);
                        if (ec) return;
                        after_field();
                        state_ = csv_state_type::between_fields;
                        goto all_csv_states;
                    }
                }
                break;
            case csv_state_type::quoted_string: 
                {
                    if (curr_char_ == parameters_.quote_escape_char())
                    {
                        state_ = csv_state_type::escaped_value;
                    }
                    else if (curr_char_ == parameters_.quote_char())
                    {
                        if (column_index_ == 0)
                        {
                            before_record();
                        }
                        end_quoted_string_value(ec);
                        if (ec) return;
                        after_field();
                        state_ = csv_state_type::between_fields;
                    }
                    else
                    {
                        value_buffer_.push_back(static_cast<CharT>(curr_char_));
                    }
                }
                break;
            case csv_state_type::unquoted_string: 
                {
                    if (curr_char_ == '\r' || (prev_char_ != '\r' && curr_char_ == '\n'))
                    {
                        if (parameters_.trim_leading() || parameters_.trim_trailing())
                        {
                            trim_string_buffer(parameters_.trim_leading(),parameters_.trim_trailing());
                        }
                        if (!parameters_.ignore_empty_lines() || (column_index_ > 0 || value_buffer_.length() > 0))
                        {
                            if (column_index_ == 0)
                            {
                                before_record();
                            }
                            end_unquoted_string_value();
                            after_field();
                            after_record();
                        }
                        state_ = csv_state_type::expect_value;
                    }
                    else if (curr_char_ == '\n')
                    {
                        if (prev_char_ != '\r')
                        {
                            if (parameters_.trim_leading() || parameters_.trim_trailing())
                            {
                                trim_string_buffer(parameters_.trim_leading(),parameters_.trim_trailing());
                            }
                            if (!parameters_.ignore_empty_lines() || (column_index_ > 0 || value_buffer_.length() > 0))
                            {
                                if (column_index_ == 0)
                                {
                                    before_record();
                                }
                                end_unquoted_string_value();
                                after_field();
                                after_record();
                            }
                            state_ = csv_state_type::expect_value;
                        }
                    }
                    else if (curr_char_ == parameters_.field_delimiter())
                    {
                        if (parameters_.trim_leading() || parameters_.trim_trailing())
                        {
                            trim_string_buffer(parameters_.trim_leading(),parameters_.trim_trailing());
                        }
                        if (column_index_ == 0)
                        {
                            before_record();
                        }
                        end_unquoted_string_value();
                        after_field();
                        state_ = csv_state_type::expect_value;
                    }
                    else if (curr_char_ == parameters_.quote_char())
                    {
                        value_buffer_.clear();
                        state_ = csv_state_type::quoted_string;
                    }
                    else
                    {
                        value_buffer_.push_back(static_cast<CharT>(curr_char_));
                    }
                }
                break;
            default:
                err_handler_.fatal_error(csv_parser_errc::invalid_state, *this);
                ec = csv_parser_errc::invalid_state;
                return;
            }
            if (line_ > parameters_.max_lines())
            {
                state_ = csv_state_type::done;
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
        std::error_code ec;
        end_parse(ec);
        if (ec)
        {
            throw parse_error(ec,line_,column_);
        }
    }

    void end_parse(std::error_code& ec)
    {
        switch (state_)
        {
        case csv_state_type::unquoted_string: 
            if (parameters_.trim_leading() || parameters_.trim_trailing())
            {
                trim_string_buffer(parameters_.trim_leading(),parameters_.trim_trailing());
            }
            if (!parameters_.ignore_empty_lines() || (column_index_ > 0 || value_buffer_.length() > 0))
            {
                if (column_index_ == 0)
                {
                    before_record();
                }
                end_unquoted_string_value();
                after_field();
            }
            break;
        case csv_state_type::escaped_value:
            if (parameters_.quote_escape_char() == parameters_.quote_char())
            {
                if (column_index_ == 0)
                {
                    before_record();
                }
                end_quoted_string_value(ec);
                if (ec) return;
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
        case csv_mode_type::header:
            pop_mode(csv_mode_type::header);
            break;
        case csv_mode_type::data:
            pop_mode(csv_mode_type::data);
            break;
        default:
            break;
        }
        if (parameters_.mapping() == mapping_type::m_columns)
        {
            handler_.begin_object(*this);
            for (size_t i = 0; i < column_values_.size(); ++i)
            {
                handler_.name(string_view_type(column_names_[i].data(),column_names_[i].size()),*this);
                handler_.begin_array(*this);
                for (const auto& val : column_values_[i])
                {
                    end_value(val,i);
                }
                handler_.end_array(*this);
            }
            handler_.end_object(*this);
        }
        else
        {
            handler_.end_array(*this);
        }
        if (!pop_mode(csv_mode_type::initial))
        {
            err_handler_.fatal_error(csv_parser_errc::unexpected_eof, *this);
            ec = csv_parser_errc::unexpected_eof;
            return;
        }
        handler_.end_json();
    }

    csv_state_type state() const
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
        size_t length = value_buffer_.length();
        if (trim_leading)
        {
            bool done = false;
            while (!done && start < value_buffer_.length())
            {
                if ((value_buffer_[start] < 256) && std::isspace(value_buffer_[start]))
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
                if ((value_buffer_[length-1] < 256) && std::isspace(value_buffer_[length-1]))
                {
                    --length;
                }
                else
                {
                    done = true;
                }
            }
        }
        if (start != 0 || length != value_buffer_.size())
        {
            value_buffer_ = value_buffer_.substr(start,length-start);
        }
    }

    void end_unquoted_string_value() 
    {
        switch (stack_[top_])
        {
        case csv_mode_type::header:
            if (parameters_.assume_header() && line_ == 1)
            {
                column_names_.push_back(value_buffer_);
            }
            break;
        case csv_mode_type::data:
            switch (parameters_.mapping())
            {
            case mapping_type::n_rows:
                if (parameters_.unquoted_empty_value_is_null() && value_buffer_.length() == 0)
                {
                    handler_.null_value(*this);
                }
                else
                {
                    end_value(value_buffer_,column_index_);
                }
                break;
            case mapping_type::n_objects:
                if (!(parameters_.ignore_empty_values() && value_buffer_.size() == 0))
                {
                    if (column_index_ < column_names_.size() + offset_)
                    {
                        handler_.name(column_names_[column_index_ - offset_], *this);
                        if (parameters_.unquoted_empty_value_is_null() && value_buffer_.length() == 0)
                        {
                            handler_.null_value(*this);
                        }
                        else
                        {
                            end_value(value_buffer_,column_index_);
                        }
                    }
                    else if (level_ > 0)
                    {
                        if (parameters_.unquoted_empty_value_is_null() && value_buffer_.length() == 0)
                        {
                            handler_.null_value(*this);
                        }
                        else
                        {
                            end_value(value_buffer_,column_index_);
                        }
                    }
                }
                break;
            case mapping_type::m_columns:
                if (column_index_ < column_values_.size())
                {
                    column_values_[column_index_].push_back(value_buffer_);
                }
                break;
            }
            break;
        default:
            break;
        }
        state_ = csv_state_type::expect_value;
        value_buffer_.clear();
    }

    void end_quoted_string_value(std::error_code& ec) 
    {
        if (parameters_.trim_leading_inside_quotes() | parameters_.trim_trailing_inside_quotes())
        {
            trim_string_buffer(parameters_.trim_leading_inside_quotes(),parameters_.trim_trailing_inside_quotes());
        }
        switch (stack_[top_])
        {
        case csv_mode_type::header:
            if (parameters_.assume_header() && line_ == 1)
            {
                column_names_.push_back(value_buffer_);
            }
            break;
        case csv_mode_type::data:
            switch (parameters_.mapping())
            {
            case mapping_type::n_rows:
                end_value(value_buffer_,column_index_);
                break;
            case mapping_type::n_objects:
                if (!(parameters_.ignore_empty_values() && value_buffer_.size() == 0))
                {
                    if (column_index_ < column_names_.size() + offset_)
                    {
                        handler_.name(column_names_[column_index_ - offset_], *this);
                        if (parameters_.unquoted_empty_value_is_null() && value_buffer_.length() == 0)
                        {
                            handler_.null_value(*this);
                        }
                        else
                        {
                            end_value(value_buffer_,column_index_);
                        }
                    }
                    else if (level_ > 0)
                    {
                        if (parameters_.unquoted_empty_value_is_null() && value_buffer_.length() == 0)
                        {
                            handler_.null_value(*this);
                        }
                        else
                        {
                            end_value(value_buffer_,column_index_);
                        }
                    }
                }
                break;
            case mapping_type::m_columns:
                break;
            }
            break;
        default:
            err_handler_.fatal_error(csv_parser_errc::invalid_csv_text, *this);
            ec = csv_parser_errc::invalid_csv_text;
            return;
        }
        state_ = csv_state_type::expect_value;
        value_buffer_.clear();
    }

    void end_value(const string_view_type& value, size_t column_index)
    {
        if (column_index < column_types_.size() + offset_)
        {
            if (column_types_[column_index - offset_].col_type == csv_column_type::repeat_t)
            {
                offset_ = offset_ + column_types_[column_index - offset_].rep_count;
                if (column_index - offset_ + 1 < column_types_.size())
                {
                    if (column_index == offset_ || level_ > column_types_[column_index-offset_].level)
                    {
                        handler_.end_array(*this);
                    }
                    level_ = column_index == offset_ ? 0 : column_types_[column_index - offset_].level;
                }
            }
            if (level_ < column_types_[column_index - offset_].level)
            {
                handler_.begin_array(*this);
                level_ = column_types_[column_index - offset_].level;
            }
            else if (level_ > column_types_[column_index - offset_].level)
            {
                handler_.end_array(*this);
                level_ = column_types_[column_index - offset_].level;
            }
            switch (column_types_[column_index - offset_].col_type)
            {
            case csv_column_type::integer_t:
                {
                std::istringstream iss{ std::string(value) };
                    int64_t val;
                    iss >> val;
                    if (!iss.fail())
                    {
                        handler_.integer_value(val, *this);
                    }
                    else
                    {
                        if (column_index - offset_ < column_defaults_.size() && column_defaults_[column_index - offset_].length() > 0)
                        {
                            std::basic_stringstream<CharT> ss(column_defaults_[column_index - offset_]);
                            basic_json_reader<CharT> reader(ss,filter_);
                            reader.read();
                        }
                        else
                        {
                            handler_.null_value(*this);
                        }
                    }
                }
                break;
            case csv_column_type::float_t:
                {
                std::istringstream iss{ std::string(value) };
                    double val;
                    iss >> val;
                    if (!iss.fail())
                    {
                        handler_.double_value(val, *this);
                    }
                    else
                    {
                        if (column_index - offset_ < column_defaults_.size() && column_defaults_[column_index - offset_].length() > 0)
                        {
                            std::basic_stringstream<CharT> ss(column_defaults_[column_index - offset_]);
                            basic_json_reader<CharT> reader(ss,filter_);
                            reader.read();
                        }
                        else
                        {
                            handler_.null_value(*this);
                        }
                    }
                }
                break;
            case csv_column_type::boolean_t:
                {
                    if (value.length() == 1 && value[0] == '0')
                    {
                        handler_.bool_value(false, *this);
                    }
                    else if (value.length() == 1 && value[0] == '1')
                    {
                        handler_.bool_value(true, *this);
                    }
                    else if (value.length() == 5 && ((value[0] == 'f' || value[0] == 'F') && (value[1] == 'a' || value[1] == 'A') && (value[2] == 'l' || value[2] == 'L') && (value[3] == 's' || value[3] == 'S') && (value[4] == 'e' || value[4] == 'E')))
                    {
                        handler_.bool_value(false, *this);
                    }
                    else if (value.length() == 4 && ((value[0] == 't' || value[0] == 'T') && (value[1] == 'r' || value[1] == 'R') && (value[2] == 'u' || value[2] == 'U') && (value[3] == 'e' || value[3] == 'E')))
                    {
                        handler_.bool_value(true, *this);
                    }
                    else
                    {
                        if (column_index - offset_ < column_defaults_.size() && column_defaults_[column_index - offset_].length() > 0)
                        {
                            std::basic_stringstream<CharT> ss(column_defaults_[column_index - offset_]);
                            basic_json_reader<CharT> reader(ss,filter_);
                            reader.read();
                        }
                        else
                        {
                            handler_.null_value(*this);
                        }
                    }
                }
                break;
            default:
                if (value.length() > 0)
                {
                    handler_.string_value(value, *this);
                }
                else
                {
                    if (column_index < column_defaults_.size() + offset_ && column_defaults_[column_index - offset_].length() > 0)
                    {
                        std::basic_stringstream<CharT> ss(column_defaults_[column_index - offset_]);
                        basic_json_reader<CharT> reader(ss,filter_);
                        reader.read();
                    }
                    else
                    {
                        handler_.string_value(string_view_type(), *this);
                    }
                }
                break;  
            }
        }
        else
        {
            handler_.string_value(value, *this);
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

    void push_mode(csv_mode_type mode)
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

    bool peek(csv_mode_type mode)
    {
        return stack_[top_] == mode;
    }

    bool flip(csv_mode_type mode1, csv_mode_type mode2)
    {
        if (top_ < 0 || stack_[top_] != mode1)
        {
            return false;
        }
        stack_[top_] = mode2;
        return true;
    }

    bool pop_mode(csv_mode_type mode)
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

