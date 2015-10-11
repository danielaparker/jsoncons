// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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

template <typename Char>
class basic_csv_parameters
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_csv_parameters()
        :
        assume_header_(false),
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

    Char field_delimiter() const
    {
        return field_delimiter_;
    }

    void field_delimiter(Char value)
    {
        field_delimiter_ = value;
    }

    std::basic_string<Char> line_delimiter() const
    {
        return line_delimiter_;
    }

    void line_delimiter(std::basic_string<Char> value)
    {
        line_delimiter_ = value;
    }

    Char quote_char() const
    {
        return quote_char_;
    }

    void quote_char(Char value)
    {
        quote_char_ = value;
    }

    Char quote_escape_char() const
    {
        return quote_escape_char_;
    }

    void quote_escape_char(Char value)
    {
        quote_escape_char_ = value;
    }

    Char comment_starter() const
    {
        return comment_starter_;
    }

    void comment_starter(Char value)
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
    Char field_delimiter_;
    std::basic_string<Char> line_delimiter_;
    Char quote_char_;
    Char quote_escape_char_;
    Char comment_starter_;
    quote_styles::quote_styles_t quote_style_;
    unsigned long max_lines_;
    size_t header_lines_;
};

typedef basic_csv_parameters<char> csv_parameters;
typedef basic_csv_parameters<wchar_t> wcsv_parameters;

}}
#endif
