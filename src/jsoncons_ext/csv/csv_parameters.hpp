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

enum class quote_styles
{
    all,minimal,none,nonnumeric
};

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

    basic_csv_parameters<CharT>& header_lines(size_t value)
    {
        header_lines_ = value;
        return *this;
    }

    bool assume_header() const
    {
        return assume_header_;
    }

    basic_csv_parameters<CharT>& assume_header(bool value)
    {
        assume_header_ = value;
        return *this;
    }

    bool ignore_empty_values() const
    {
        return ignore_empty_values_;
    }

    basic_csv_parameters<CharT>& ignore_empty_values(bool value)
    {
        ignore_empty_values_ = value;
        return *this;
    }

    bool trim_leading() const
    {
        return trim_leading_;
    }

    basic_csv_parameters<CharT>& trim_leading(bool value)
    {
        trim_leading_ = value;
        return *this;
    }

    bool trim_trailing() const
    {
        return trim_trailing_;
    }

    basic_csv_parameters<CharT>& trim_trailing(bool value)
    {
        trim_trailing_ = value;
        return *this;
    }

    bool trim_leading_inside_quotes() const
    {
        return trim_leading_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_leading_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        return *this;
    }

    bool trim_trailing_inside_quotes() const
    {
        return trim_trailing_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_trailing_inside_quotes(bool value)
    {
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool trim() const
    {
        return trim_leading_ && trim_trailing_;
    }

    basic_csv_parameters<CharT>& trim(bool value)
    {
        trim_leading_ = value;
        trim_trailing_ = value;
        return *this;
    }

    bool trim_inside_quotes() const
    {
        return trim_leading_inside_quotes_ && trim_trailing_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool unquoted_empty_value_is_null() const
    {
        return unquoted_empty_value_is_null_;
    }

    basic_csv_parameters<CharT>& unquoted_empty_value_is_null(bool value)
    {
        unquoted_empty_value_is_null_ = value;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_names() const
    {
        return column_names_;
    }

    basic_csv_parameters<CharT>& column_names(const std::vector<std::basic_string<CharT>>& value)
    {
        column_names_ = value;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_types() const
    {
        return column_types_;
    }

    basic_csv_parameters<CharT>& column_types(const std::vector<std::basic_string<CharT>>& value)
    {
        column_types_ = value;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_defaults() const
    {
        return column_defaults_;
    }

    basic_csv_parameters<CharT>& column_defaults(const std::vector<std::basic_string<CharT>>& value)
    {
        column_defaults_ = value;
        return *this;
    }

    CharT field_delimiter() const
    {
        return field_delimiter_;
    }

    basic_csv_parameters<CharT>& field_delimiter(CharT value)
    {
        field_delimiter_ = value;
        return *this;
    }

    std::basic_string<CharT> line_delimiter() const
    {
        return line_delimiter_;
    }

    basic_csv_parameters<CharT>& line_delimiter(std::basic_string<CharT> value)
    {
        line_delimiter_ = value;
        return *this;
    }

    CharT quote_char() const
    {
        return quote_char_;
    }

    basic_csv_parameters<CharT>& quote_char(CharT value)
    {
        quote_char_ = value;
        return *this;
    }

    CharT quote_escape_char() const
    {
        return quote_escape_char_;
    }

    basic_csv_parameters<CharT>& quote_escape_char(CharT value)
    {
        quote_escape_char_ = value;
        return *this;
    }

    CharT comment_starter() const
    {
        return comment_starter_;
    }

    basic_csv_parameters<CharT>& comment_starter(CharT value)
    {
        comment_starter_ = value;
        return *this;
    }

    quote_styles quote_style() const
    {
        return quote_style_;
    }

    basic_csv_parameters<CharT>& assume_header(quote_styles value)
    {
        quote_style_ = value;
        return *this;
    }

    unsigned long max_lines() const
    {
        return max_lines_;
    }

    basic_csv_parameters<CharT>& max_lines(unsigned long value)
    {
        max_lines_ = value;
        return *this;
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    std::basic_string<CharT> header() const
    {
        return header_;
    }

    basic_csv_parameters<CharT>& header(const std::basic_string<CharT>& value)
    {
        header_ = value;
        return *this;
    }

    std::basic_string<CharT> data_types() const
    {
        return data_types_;
    }

    basic_csv_parameters<CharT>& data_types(const std::basic_string<CharT>& value)
    {
        data_types_ = value;
        return *this;
    }

    std::basic_string<CharT> default_values() const
    {
        return default_values_;
    }

    basic_csv_parameters<CharT>& default_values(const std::basic_string<CharT>& value)
    {
        default_values_ = value;
        return *this;
    }
#endif
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
    quote_styles quote_style_;
    unsigned long max_lines_;
    size_t header_lines_;
    std::basic_string<CharT> line_delimiter_;
    std::basic_string<CharT> header_;
    std::basic_string<CharT> data_types_;
    std::basic_string<CharT> default_values_;
    std::vector<std::basic_string<CharT>> column_names_;
    std::vector<std::basic_string<CharT>> column_types_;
    std::vector<std::basic_string<CharT>> column_defaults_;
};

typedef basic_csv_parameters<char> csv_parameters;
typedef basic_csv_parameters<wchar_t> wcsv_parameters;

}}
#endif
