// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

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

template <typename CharT>
class buffered_ostream;

template <typename CharT>
class basic_output_format
{
    int indent_;
    int precision_;
    bool replace_nan_;
    bool replace_pos_inf_;
    bool replace_neg_inf_;
    std::basic_string<CharT> nan_replacement_;
    std::basic_string<CharT> pos_inf_replacement_;
    std::basic_string<CharT> neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_output_format()
        :
        indent_(default_indent),
        precision_(15),
        replace_nan_(true),
        replace_pos_inf_(true),
        replace_neg_inf_(true),
        nan_replacement_(json_char_traits<CharT,sizeof(CharT)>::null_literal()),
        pos_inf_replacement_(json_char_traits<CharT,sizeof(CharT)>::null_literal()),
        neg_inf_replacement_(json_char_traits<CharT,sizeof(CharT)>::null_literal()),
        escape_all_non_ascii_(false),
        escape_solidus_(false)
    {
    }

//  Accessors

    int indent() const
    {
        return indent_;
    }

    int precision() const
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

    std::basic_string<CharT> nan_replacement() const
    {
        return nan_replacement_;
    }

    std::basic_string<CharT> pos_inf_replacement() const
    {
        return pos_inf_replacement_;
    }

    std::basic_string<CharT> neg_inf_replacement() const
    {
        return neg_inf_replacement_;
    }

//  Modifiers

    void precision(int prec)
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

    void nan_replacement(const std::basic_string<CharT>& replacement)
    {
        nan_replacement_ = replacement;
    }

    void pos_inf_replacement(const std::basic_string<CharT>& replacement)
    {
        pos_inf_replacement_ = replacement;
    }

    void neg_inf_replacement(const std::basic_string<CharT>& replacement)
    {
        neg_inf_replacement_ = replacement;
    }

    void indent(int value)
    {
        indent_ = value;
    }
};

template<typename CharT>
void escape_string(const CharT* s,
                   size_t length,
                   const basic_output_format<CharT>& format,
                   buffered_ostream<CharT>& os)
{
    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '\\':
            os.put('\\'); 
            os.put('\\');
            break;
        case '"':
            os.put('\\'); 
            os.put('\"');
            break;
        case '\b':
            os.put('\\'); 
            os.put('b');
            break;
        case '\f':
            os.put('\\');
            os.put('f');
            break;
        case '\n':
            os.put('\\');
            os.put('n');
            break;
        case '\r':
            os.put('\\');
            os.put('r');
            break;
        case '\t':
            os.put('\\');
            os.put('t');
            break;
        default:
            uint32_t u(c >= 0 ? c : 256 + c);
            if (format.escape_solidus() && c == '/')
            {
                os.put('\\');
                os.put('/');
            }
            else if (is_control_character(u) || format.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                uint32_t cp = json_char_traits<CharT, sizeof(CharT)>::convert_char_to_codepoint(it, end);
                if (is_non_ascii_character(cp) || is_control_character(u))
                {
                    if (cp > 0xFFFF)
                    {
                        cp -= 0x10000;
                        uint32_t first = (cp >> 10) + 0xD800;
                        uint32_t second = ((cp & 0x03FF) + 0xDC00);

                        os.put('\\');
                        os.put('u');
                        os.put(to_hex_character(first >> 12 & 0x000F));
                        os.put(to_hex_character(first >> 8  & 0x000F));
                        os.put(to_hex_character(first >> 4  & 0x000F));
                        os.put(to_hex_character(first     & 0x000F));
                        os.put('\\');
                        os.put('u');
                        os.put(to_hex_character(second >> 12 & 0x000F));
                        os.put(to_hex_character(second >> 8  & 0x000F));
                        os.put(to_hex_character(second >> 4  & 0x000F));
                        os.put(to_hex_character(second     & 0x000F));
                    }
                    else
                    {
                        os.put('\\');
                        os.put('u');
                        os.put(to_hex_character(cp >> 12 & 0x000F));
                        os.put(to_hex_character(cp >> 8  & 0x000F));
                        os.put(to_hex_character(cp >> 4  & 0x000F));
                        os.put(to_hex_character(cp     & 0x000F));
                    }
                }
                else
                {
                    os.put(c);
                }
            }
            else if (format.escape_solidus() && c == '/')
            {
                os.put('\\');
                os.put('/');
            }
            else
            {
                os.put(c);
            }
            break;
        }
    }
}

typedef basic_output_format<char> output_format;
typedef basic_output_format<wchar_t> woutput_format;

}
#endif
