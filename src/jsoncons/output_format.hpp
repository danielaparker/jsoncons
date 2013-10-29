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

namespace jsoncons {

template <class Char>
class basic_output_format
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_output_format()
        : indenting_(false), 
          indent_(default_indent),
          precision_(16),
          set_format_flags_(0),
          unset_format_flags_(0),
          replace_nan_(true),replace_pos_inf_(true),replace_neg_inf_(true), 
          pos_inf_replacement_(json_char_traits<Char>::null_literal()),
          neg_inf_replacement_(json_char_traits<Char>::null_literal()),
          nan_replacement_(json_char_traits<Char>::null_literal()),
          escape_all_non_ascii_(false),
          escape_solidus_(false)
    {
    }

    basic_output_format(bool indenting)
        : indenting_(indenting), 
          precision_(16),
          set_format_flags_(0),
          unset_format_flags_(0),
          indent_(default_indent),
          replace_nan_(true),replace_pos_inf_(true),replace_neg_inf_(true), 
          pos_inf_replacement_(json_char_traits<Char>::null_literal()),
          neg_inf_replacement_(json_char_traits<Char>::null_literal()),
          nan_replacement_(json_char_traits<Char>::null_literal()),
          escape_all_non_ascii_(false),
          escape_solidus_(false)
    {
    }

//  Accessors

    bool indenting() const
    {
        return indenting_;
    }

    size_t indent() const
    {
        return indent_;
    }

    std::streamsize precision() const 
    {
        return precision_; 
    }

    std::ios_base::fmtflags set_format_flags() const
    {
        return set_format_flags_;
    }

    std::ios_base::fmtflags unset_format_flags() const
    {
        return unset_format_flags_;
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

    void indenting(bool value)
    {
        indenting_ = value;
    }

    void indent(size_t value)
    {
        indent_ = value;
    }

    void precision(std::streamsize prec)
    {
        precision_ = prec; 
    }

    void set_format_flags(std::ios_base::fmtflags flags)
    {
        set_format_flags_ = flags;
    }

    void unset_format_flags(std::ios_base::fmtflags flags)
    {
        unset_format_flags_ = flags;
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

    void fixed_decimal_places(std::streamsize n)
    {
        set_format_flags_ |= std::ios::fixed;
        precision_ = n;
    }
private:
    bool indenting_;
    size_t indent_;
    std::streamsize precision_;
    std::ios_base::fmtflags set_format_flags_;
    std::ios_base::fmtflags unset_format_flags_;

    bool replace_nan_;
    bool replace_pos_inf_;
    bool replace_neg_inf_;
    std::basic_string<Char> nan_replacement_;
    std::basic_string<Char> pos_inf_replacement_;
    std::basic_string<Char> neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
    size_t max_decimal_places;
};

typedef basic_output_format<char> output_format;

}
#endif
