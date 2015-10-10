// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons_ext/csv/csv_text_error_category.hpp"
#include "jsoncons_ext/csv/csv_parameters.hpp"

namespace jsoncons { namespace csv {

namespace modes {
    enum modes_t {
        done,
        header,
		array,
		object
    };
};

namespace states {
    enum states_t {
        start, 
        between_fields,
		quoted_string,
		unquoted_string,
        escaped_value,
        done,
        error
    };
};

template<typename Char>
class basic_csv_parser : private basic_parsing_context<Char>
{
    static const int default_depth = 100;

public:
    basic_csv_parser(basic_json_input_handler<Char>& handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
		cr_ = false;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    basic_csv_parser(basic_json_input_handler<Char>& handler,
                     basic_csv_parameters<Char> params)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         parameters_(params),
         err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
        cr_ = false;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    basic_csv_parser(basic_json_input_handler<Char>& handler,
                     basic_parse_error_handler<Char>& err_handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
        cr_ = false;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    basic_csv_parser(basic_json_input_handler<Char>& handler,
                     basic_parse_error_handler<Char>& err_handler,
                     basic_csv_parameters<Char> params)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         parameters_(params),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        column_index_ = 0;
        cr_ = false;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    ~basic_csv_parser()
    {
    }

    basic_parsing_context<Char> const & parsing_context() const
    {
        return *this;
    }

    bool done() const
    {
        return state_ == states::done;
    }

    void begin_parse()
    {
        push(modes::done);
        handler_->begin_json();
        if (parameters_.assume_header())
        {
            if (!push(modes::header))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
            }
        }
        else
        {
            if (!push(modes::array))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
            }
        }
        handler_->begin_array(*this);
        state_ = states::unquoted_string;
        column_index_ = 0;
    }

    void parse(Char const* p, size_t start, size_t length)
    {
        index_ = start;
        for (; index_ < length && state_ != states::done; ++index_)
        {
            bool is_newline = false;
            int next_char = p[index_];
            switch (next_char)
            {
            case '\r':
                ++line_;
                column_ = 0;
                cr_ = true;
                is_newline = true;
                break;
            case '\n':
                if (!cr_)
                {
                    ++line_;
                    column_ = 0;
                    is_newline = true;
                }
                else
                {
                    cr_ = false;
                }
                break;
            default:
                cr_ = false;
                break;
            }
            ++column_;

            switch (state_)
            {
            case states::between_fields:
                if (is_newline)
                {
                    switch (stack_[top_])
                    {
                    case modes::array:
                        handler_->end_array(*this);
                        break;
                    case modes::object:
                        handler_->end_object(*this);
                        break;
                    case modes::header:
                        flip(modes::header, modes::object);
                        break;
                    }
                    column_index_ = 0;
                    state_ = states::unquoted_string;
                }
                else if (next_char == parameters_.field_delimiter())
                {
                    state_ = states::unquoted_string;
                    ++column_index_;
                }
                break;
            case states::escaped_value: 
                {
                    if (next_char == parameters_.quote_char())
                    {
                        string_buffer_.push_back(next_char);
                        state_ = states::quoted_string;
                    }
                    else if (parameters_.quote_escape_char() == parameters_.quote_char())
                    {
                        if (column_index_ == 0)
                        {
                            switch (stack_[top_])
                            {
                            case modes::array:
                                handler_->begin_array(*this);
                                break;
                            case modes::object:
                                handler_->begin_object(*this);
                                break;
                            }
                        }
                        end_string_value();
                        state_ = states::between_fields;
                    }
                }
                break;
            case states::quoted_string: 
                {
                    if (next_char == parameters_.quote_escape_char())
                    {
                        state_ = states::escaped_value;
                    }
                    else if (next_char == parameters_.quote_char())
                    {
                        if (column_index_ == 0)
                        {
                            switch (stack_[top_])
                            {
                            case modes::array:
                                handler_->begin_array(*this);
                                break;
                            case modes::object:
                                handler_->begin_object(*this);
                                break;
                            }
                        }
                        end_string_value();
                        state_ = states::between_fields;
                    }
                    else
                    {
                        string_buffer_.push_back(next_char);
                    }
                }
                break;
            case states::unquoted_string: 
                {
                    if (is_newline)
                    {
                        if (column_index_ == 0)
                        {
                            switch (stack_[top_])
                            {
                            case modes::array:
                                handler_->begin_array(*this);
                                break;
                            case modes::object:
                                handler_->begin_object(*this);
                                break;
                            }
                        }
                        end_string_value();
                        switch (stack_[top_])
                        {
                        case modes::array:
                            handler_->end_array(*this);
                            break;
                        case modes::object:
                            handler_->end_object(*this);
                            break;
                        case modes::header:
                            flip(modes::header, modes::object);
                            break;
                        }
                        column_index_ = 0;
                    }
                    else if (next_char == parameters_.quote_char())
                    {
                        string_buffer_.clear();
                        state_ = states::quoted_string;
                    }
                    else if (next_char == parameters_.field_delimiter())
                    {
                        if (column_index_ == 0)
                        {
                            switch (stack_[top_])
                            {
                            case modes::array:
                                handler_->begin_array(*this);
                                break;
                            case modes::object:
                                handler_->begin_object(*this);
                                break;
                            }
                        }
                        end_string_value();
                        ++column_index_;
                    }
                    else
                    {
                        string_buffer_.push_back(next_char);
                    }
                }
                break;
            }
            if (line_ > parameters_.max_lines())
            {
                state_ = states::done;
            }
        }
    }

    void end_parse()
    {
		if (column_index_ > 0)
		{
            switch (stack_[top_])
            {
            case modes::array:
                handler_->end_array(*this);
                break;
            case modes::object:
                handler_->end_object(*this);
                break;
            case modes::header:
                flip(modes::header, modes::object);
                break;
            }
            column_index_ = 0;
		}
        switch (stack_[top_])
        {
        case modes::array:
            if (!pop(modes::array))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_text_error_category()), *this);
            }
            break;
        case modes::object:
            if (!pop(modes::object))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_text_error_category()), *this);
            }
            break;
        case modes::header:
            if (!pop(modes::header))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_text_error_category()), *this);
            }
            break;
        }
        handler_->end_array(*this);
        if (!pop(modes::done))
        {
            state_ = states::error;
            err_handler_->error(std::error_code(csv_parser_errc::unexpected_eof, csv_text_error_category()), *this);
        }
        handler_->end_json();
    }

    states::states_t state() const
    {
        return state_;
    }

    size_t index() const
    {
        return index_;
    }
private:

    void end_string_value() 
    {
        switch (stack_[top_])
        {
        case modes::header:
            column_labels_.push_back(string_buffer_);
            state_ = states::unquoted_string;
            break;
        case modes::object:
            if (column_index_ < column_labels_.size())
            {
                handler_->name(column_labels_[column_index_].c_str(), column_labels_[column_index_].length(), *this);
                handler_->value(string_buffer_.c_str(), string_buffer_.length(), *this);
            }
            state_ = states::unquoted_string;
            break;
        case modes::array:
            handler_->value(string_buffer_.c_str(), string_buffer_.length(), *this);
            state_ = states::unquoted_string;
            break;
        default:
            state_ = states::error;
            err_handler_->error(std::error_code(csv_parser_errc::invalid_csv_text, csv_text_error_category()), *this);
            break;
        }
        string_buffer_.clear();
    }

    void begin_member_or_element() 
    {
        switch (stack_[top_])
        {
        case modes::object_member_value:
            // A comma causes a flip from object_member_value modes to object_member_name modes.
            if (!flip(modes::object_member_value, modes::object_member_name))
            {
                state_ = states::error;
                err_handler_->error(std::error_code(csv_parser_errc::invalid_json_text, csv_text_error_category()), *this);
            }
            state_ = states::expect_member_name;
            break;
        case modes::array_element:
            state_ = states::expect_value;
            break;
        default:
            state_ = states::error;
            err_handler_->error(std::error_code(csv_parser_errc::invalid_json_text, csv_text_error_category()), *this);
            break;
        }
    }

    void begin_member_value()
    {
        if (!flip(modes::object_member_name, modes::object_member_value))
        {
            state_ = states::error;
            err_handler_->error(std::error_code(csv_parser_errc::invalid_json_text, csv_text_error_category()), *this);
        }
        state_ = states::expect_value;
    }

    unsigned long do_line_number() const override
    {
        return line_;
    }

    unsigned long do_column_number() const override
    {
        return column_;
    }

    Char do_last_char() const override
    {
        return c_;
    }

    bool push(modes::modes_t modes)
    {
        ++top_;
        if (top_ >= depth_)
        {
            if (top_ >= max_depth_)
            {
                return false;
            }
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_] = modes;
        return true;
    }

    int peek()
    {
        return stack_[top_];
    }

    bool peek(modes::modes_t modes)
    {
        return stack_[top_] == modes;
    }

    bool flip(modes::modes_t mode1, modes::modes_t mode2)
    {
        if (top_ < 0 || stack_[top_] != mode1)
        {
            return false;
        }
        stack_[top_] = mode2;
        return true;
    }

    bool pop(modes::modes_t modes)
    {
        if (top_ < 0 || stack_[top_] != modes)
        {
            return false;
        }
        --top_;
        return true;
    }

    template<typename CharType>
    unsigned long long string_to_unsigned(const CharType *s, size_t length) throw(std::overflow_error)
    {
        const unsigned long long max_value = std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        const unsigned long long max_value_div_10 = max_value / 10;
        unsigned long long n = 0;
        for (size_t i = 0; i < length; ++i)
        {
            unsigned long long x = s[i] - '0';
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

    template<typename CharType>
    long long string_to_integer(bool has_neg, const CharType *s, size_t length) throw(std::overflow_error)
    {
        const long long max_value = std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
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

    states::states_t state_;
    int top_;
    std::vector<modes::modes_t> stack_;
    basic_json_input_handler<Char> *handler_;
    basic_parse_error_handler<Char> *err_handler_;
    unsigned long column_;
    unsigned long line_;
    Char c_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<Char> string_buffer_;
    bool is_negative_;
    states::states_t saved_state_;
    bool cr_;
    size_t index_;
    int depth_;
    int max_depth_;
    basic_csv_parameters<Char> parameters_;
    std::vector<std::basic_string<Char>> column_labels_;
	size_t column_index_;
};

typedef basic_csv_parser<char> csv_parser;
typedef basic_csv_parser<wchar_t> wcsv_parser;

}}

#endif

