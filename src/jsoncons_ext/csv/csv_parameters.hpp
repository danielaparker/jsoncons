// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_PARAMETERS_HPP
#define JSONCONS_CSV_CSV_PARAMETERS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>

namespace jsoncons { namespace csv {

namespace quote_styles
{
    enum quote_styles_t{all,minimal,none,nonnumeric};
};

    namespace json_types
    {
        enum json_types_t{string_t,number_t,bool_t};
    }

template <typename CharT>
class basic_csv_parameters
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_csv_parameters()
        :
        assume_header_(false),
        ignore_empty_values_(false),
        trim_leading_(false),
        trim_trailing_(false),
        trim_leading_inside_quotes_(false),
        trim_trailing_inside_quotes_(false),
        unquoted_empty_value_is_null_(false),
        field_delimiter_(','),
        quote_char_('\"'),
        quote_escape_char_('\"'),
        comment_starter_('\0'),
        quote_style_(quote_styles::minimal),
        max_lines_(std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP()),
        header_lines_(0)
    {
        line_delimiter_.push_back('\n');
    }

//  Properties

    size_t header_lines() const
    {
        return (assume_header_ && header_lines_ <= 1) ? 1 : header_lines_;
    }

    void header_lines(size_t value)
    {
        header_lines_ = value;
    }

    bool assume_header() const
    {
        return assume_header_;
    }

    void assume_header(bool value)
    {
        assume_header_ = value;
    }

    bool ignore_empty_values() const
    {
        return ignore_empty_values_;
    }

    void ignore_empty_values(bool value)
    {
        ignore_empty_values_ = value;
    }

    bool trim_leading() const
    {
        return trim_leading_;
    }

    void trim_leading(bool value)
    {
        trim_leading_ = value;
    }

    bool trim_trailing() const
    {
        return trim_trailing_;
    }

    void trim_trailing(bool value)
    {
        trim_trailing_ = value;
    }

    bool trim_leading_inside_quotes() const
    {
        return trim_leading_inside_quotes_;
    }

    void trim_leading_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
    }

    bool trim_trailing_inside_quotes() const
    {
        return trim_trailing_inside_quotes_;
    }

    void trim_trailing_inside_quotes(bool value)
    {
        trim_trailing_inside_quotes_ = value;
    }

    bool trim() const
    {
        return trim_leading_ && trim_trailing_;
    }

    void trim(bool value)
    {
        trim_leading_ = value;
        trim_trailing_ = value;
    }

    bool trim_inside_quotes() const
    {
        return trim_leading_inside_quotes_ && trim_trailing_inside_quotes_;
    }

    void trim_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        trim_trailing_inside_quotes_ = value;
    }

    bool unquoted_empty_value_is_null() const
    {
        return unquoted_empty_value_is_null_;
    }

    void unquoted_empty_value_is_null(bool value)
    {
        unquoted_empty_value_is_null_ = value;
    }

    std::basic_string<CharT> data_types() const
    {
        return data_types_;
    }

    std::basic_string<CharT> header() const
    {
        return header_;
    }

    void header(std::basic_string<CharT> value)
    {
        header_ = value;
    }

    void data_types(std::basic_string<CharT> value)
    {
        data_types_ = value;
    }

    CharT field_delimiter() const
    {
        return field_delimiter_;
    }

    void field_delimiter(CharT value)
    {
        field_delimiter_ = value;
    }

    std::basic_string<CharT> line_delimiter() const
    {
        return line_delimiter_;
    }

    void line_delimiter(std::basic_string<CharT> value)
    {
        line_delimiter_ = value;
    }

    CharT quote_char() const
    {
        return quote_char_;
    }

    void quote_char(CharT value)
    {
        quote_char_ = value;
    }

    CharT quote_escape_char() const
    {
        return quote_escape_char_;
    }

    void quote_escape_char(CharT value)
    {
        quote_escape_char_ = value;
    }

    CharT comment_starter() const
    {
        return comment_starter_;
    }

    void comment_starter(CharT value)
    {
        comment_starter_ = value;
    }

    quote_styles::quote_styles_t quote_style() const
    {
        return quote_style_;
    }

    void assume_header(quote_styles::quote_styles_t value)
    {
        quote_style_ = value;
    }

    unsigned long max_lines() const
    {
        return max_lines_;
    }

    void max_lines(unsigned long value)
    {
        max_lines_ = value;
    }
private:
    bool assume_header_;
    bool ignore_empty_values_;
    bool trim_leading_;
    bool trim_trailing_;
    bool trim_leading_inside_quotes_;
    bool trim_trailing_inside_quotes_;
    bool unquoted_empty_value_is_null_;
    CharT field_delimiter_;
    CharT quote_char_;
    CharT quote_escape_char_;
    CharT comment_starter_;
    quote_styles::quote_styles_t quote_style_;
    unsigned long max_lines_;
    size_t header_lines_;
    std::basic_string<CharT> line_delimiter_;
    std::basic_string<CharT> header_;
    std::basic_string<CharT> data_types_;
};

typedef basic_csv_parameters<char> csv_parameters;
typedef basic_csv_parameters<wchar_t> wcsv_parameters;

}}
#endif
