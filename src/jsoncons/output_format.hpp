// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_OUTPUT_FORMAT_HPP
#define JSONCONS_OUTPUT_FORMAT_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>

namespace jsoncons {

template <typename Char>
class basic_output_format
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_output_format()
        :
        indent_(default_indent),
        precision_(16),
        floatfield_(std::ios_base::fmtflags(0)),
        replace_nan_(true),
        replace_pos_inf_(true),
        replace_neg_inf_(true),
        nan_replacement_(json_char_traits<Char,sizeof(Char)>::null_literal()),
        pos_inf_replacement_(json_char_traits<Char,sizeof(Char)>::null_literal()),
        neg_inf_replacement_(json_char_traits<Char,sizeof(Char)>::null_literal()),
        escape_all_non_ascii_(false),
        escape_solidus_(false)
    {
    }

//  Accessors

    size_t indent() const
    {
        return indent_;
    }

    size_t precision() const
    {
        return precision_;
    }

    bool escape_all_non_ascii() const
    {
        return escape_all_non_ascii_;
    }

    bool escape_solidus() const
    {
        return escape_solidus_;
    }

    bool replace_nan() const {return replace_nan_;}

    bool replace_pos_inf() const {return replace_pos_inf_;}

    bool replace_neg_inf() const {return replace_neg_inf_;}

    std::basic_string<Char> nan_replacement() const
    {
        return nan_replacement_;
    }

    std::basic_string<Char> pos_inf_replacement() const
    {
        return pos_inf_replacement_;
    }

    std::basic_string<Char> neg_inf_replacement() const
    {
        return neg_inf_replacement_;
    }

//  Modifiers

    void precision(size_t prec)
    {
        precision_ = prec;
    }

    void escape_all_non_ascii(bool value)
    {
        escape_all_non_ascii_ = value;
    }

    void escape_solidus(bool value)
    {
        escape_solidus_ = value;
    }

    void replace_nan(bool replace)
    {
        replace_nan_ = replace;
    }

    void replace_inf(bool replace)
    {
        replace_pos_inf_ = replace;
        replace_neg_inf_ = replace;
    }

    void replace_pos_inf(bool replace)
    {
        replace_pos_inf_ = replace;
    }

    void replace_neg_inf(bool replace)
    {
        replace_neg_inf_ = replace;
    }

    void nan_replacement(const std::basic_string<Char>& replacement)
    {
        nan_replacement_ = replacement;
    }

    void pos_inf_replacement(const std::basic_string<Char>& replacement)
    {
        pos_inf_replacement_ = replacement;
    }

    void neg_inf_replacement(const std::basic_string<Char>& replacement)
    {
        neg_inf_replacement_ = replacement;
    }

    std::ios_base::fmtflags floatfield() const
    {
        return floatfield_;
    }

    void floatfield(std::ios_base::fmtflags flags)
    {
        floatfield_ = flags;
    }

    void indent(size_t value)
    {
        indent_ = value;
    }
private:
    size_t indent_;
    size_t precision_;
    std::ios_base::fmtflags floatfield_;

    bool replace_nan_;
    bool replace_pos_inf_;
    bool replace_neg_inf_;
    std::basic_string<Char> nan_replacement_;
    std::basic_string<Char> pos_inf_replacement_;
    std::basic_string<Char> neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
};

typedef basic_output_format<char> output_format;
typedef basic_output_format<wchar_t> woutput_format;

}
#endif
