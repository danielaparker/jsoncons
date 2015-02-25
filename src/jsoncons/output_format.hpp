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

template<typename Char>
void escape_string(const Char* s,
                   size_t length,
                   const basic_output_format<Char>& format,
                   std::basic_ostream<Char>& os)
{
    const Char* begin = s;
    const Char* end = s + length;
    for (const Char* it = begin; it != end; ++it)
    {
        Char c = *it;
        switch (c)
        {
        case '\\':
            os << '\\' << '\\';
            break;
        case '"':
            os << '\\' << '\"';
            break;
        case '\b':
            os << '\\' << 'b';
            break;
        case '\f':
            os << '\\';
            os << 'f';
            break;
        case '\n':
            os << '\\';
            os << 'n';
            break;
        case '\r':
            os << '\\';
            os << 'r';
            break;
        case '\t':
            os << '\\';
            os << 't';
            break;
        default:
            uint32_t u(c >= 0 ? c : 256 + c);
            if (format.escape_solidus() && c == '/')
            {
                os << '\\';
                os << '/';
            }
            else if (is_control_character(u) || format.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                uint32_t cp = json_char_traits<Char, sizeof(Char)>::convert_char_to_codepoint(it, end);
                if (is_non_ascii_character(cp) || is_control_character(u))
                {
                    if (cp > 0xFFFF)
                    {
                        cp -= 0x10000;
                        uint32_t first = (cp >> 10) + 0xD800;
                        uint32_t second = ((cp & 0x03FF) + 0xDC00);

                        os << '\\';
                        os << 'u';
                        os << to_hex_character(first >> 12 & 0x000F);
                        os << to_hex_character(first >> 8  & 0x000F);
                        os << to_hex_character(first >> 4  & 0x000F);
                        os << to_hex_character(first     & 0x000F);
                        os << '\\';
                        os << 'u';
                        os << to_hex_character(second >> 12 & 0x000F);
                        os << to_hex_character(second >> 8  & 0x000F);
                        os << to_hex_character(second >> 4  & 0x000F);
                        os << to_hex_character(second     & 0x000F);
                    }
                    else
                    {
                        os << '\\';
                        os << 'u';
                        os << to_hex_character(cp >> 12 & 0x000F);
                        os << to_hex_character(cp >> 8  & 0x000F);
                        os << to_hex_character(cp >> 4  & 0x000F);
                        os << to_hex_character(cp     & 0x000F);
                    }
                }
                else
                {
                    os << c;
                }
            }
            else if (format.escape_solidus() && c == '/')
            {
                os << '\\';
                os << '/';
            }
            else
            {
                os << c;
            }
            break;
        }
    }
}

typedef basic_output_format<char> output_format;
typedef basic_output_format<wchar_t> woutput_format;

}
#endif
