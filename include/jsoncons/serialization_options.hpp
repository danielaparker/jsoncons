// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SERIALIZATION_OPTIONS_HPP
#define JSONCONS_SERIALIZATION_OPTIONS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/jsoncons_utilities.hpp>

namespace jsoncons {

#if !defined(JSONCONS_NO_DEPRECATED)
enum class block_options {next_line,same_line};
#endif

template <class CharT>
class buffered_output;

enum class line_split_kind{same_line,new_line,multi_line};

template <class CharT>
class basic_serialization_options
{
    int indent_;
    uint8_t precision_;
    bool replace_nan_;
    bool replace_pos_inf_;
    bool replace_neg_inf_;
    std::basic_string<CharT> nan_replacement_;
    std::basic_string<CharT> pos_inf_replacement_;
    std::basic_string<CharT> neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;

    line_split_kind object_object_split_lines_;
    line_split_kind object_array_split_lines_;
    line_split_kind array_array_split_lines_;
    line_split_kind array_object_split_lines_;
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_serialization_options()
        :
        indent_(default_indent),
        precision_(0),
        replace_nan_(true),
        replace_pos_inf_(true),
        replace_neg_inf_(true),
        nan_replacement_(detail::null_literal<CharT>()),
        pos_inf_replacement_(detail::null_literal<CharT>()),
        neg_inf_replacement_(detail::null_literal<CharT>()),
        escape_all_non_ascii_(false),
        escape_solidus_(false),
        object_object_split_lines_(line_split_kind::multi_line),
        object_array_split_lines_(line_split_kind::same_line),
        array_array_split_lines_(line_split_kind::new_line),
        array_object_split_lines_(line_split_kind::multi_line)
    {
    }

//  Accessors
    line_split_kind object_object_split_lines() const {return object_object_split_lines_;}
    line_split_kind array_object_split_lines() const {return array_object_split_lines_;}
    line_split_kind object_array_split_lines() const {return object_array_split_lines_;}
    line_split_kind array_array_split_lines() const {return array_array_split_lines_;}

    basic_serialization_options<CharT>& object_object_split_lines(line_split_kind value) {object_object_split_lines_ = value; return *this;}
    basic_serialization_options<CharT>& array_object_split_lines(line_split_kind value) {array_object_split_lines_ = value; return *this;}
    basic_serialization_options<CharT>& object_array_split_lines(line_split_kind value) {object_array_split_lines_ = value; return *this;}
    basic_serialization_options<CharT>& array_array_split_lines(line_split_kind value) {array_array_split_lines_ = value; return *this;}

#if !defined(JSONCONS_NO_DEPRECATED)
    block_options array_array_block_option()
    {
        return (array_array_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_serialization_options<CharT>& array_array_block_option(block_options value)
    {
        array_array_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options array_object_block_option()
    {
        return (array_object_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_serialization_options<CharT>& array_object_block_option(block_options value)
    {
        array_object_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options object_array_block_option()
    {
        return (object_array_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_serialization_options<CharT>& object_array_block_option(block_options value)
    {
        object_array_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options object_object_block_option()
    {
        return (object_object_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_serialization_options<CharT>& object_object_block_option(block_options value)
    {
        object_object_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }
#endif

    int indent() const
    {
        return indent_;
    }

    uint8_t precision() const
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

    basic_serialization_options<CharT>& precision(uint8_t prec)
    {
        precision_ = prec;
        return *this;
    }

    basic_serialization_options<CharT>& escape_all_non_ascii(bool value)
    {
        escape_all_non_ascii_ = value;
        return *this;
    }

    basic_serialization_options<CharT>& escape_solidus(bool value)
    {
        escape_solidus_ = value;
        return *this;
    }

    basic_serialization_options<CharT>& replace_nan(bool replace)
    {
        replace_nan_ = replace;
        return *this;
    }

    basic_serialization_options<CharT>& replace_inf(bool replace)
    {
        replace_pos_inf_ = replace;
        replace_neg_inf_ = replace;
        return *this;
    }

    basic_serialization_options<CharT>& replace_pos_inf(bool replace)
    {
        replace_pos_inf_ = replace;
        return *this;
    }

    basic_serialization_options<CharT>& replace_neg_inf(bool replace)
    {
        replace_neg_inf_ = replace;
        return *this;
    }

    basic_serialization_options<CharT>& nan_replacement(const std::basic_string<CharT>& replacement)
    {
        nan_replacement_ = replacement;
        return *this;
    }

    basic_serialization_options<CharT>& pos_inf_replacement(const std::basic_string<CharT>& replacement)
    {
        pos_inf_replacement_ = replacement;
        return *this;
    }

    basic_serialization_options<CharT>& neg_inf_replacement(const std::basic_string<CharT>& replacement)
    {
        neg_inf_replacement_ = replacement;
        return *this;
    }

    basic_serialization_options<CharT>& indent(int value)
    {
        indent_ = value;
        return *this;
    }
};

template<class CharT>
void escape_string(const CharT* s,
                   size_t length,
                   const basic_serialization_options<CharT>& options,
                   buffered_output<CharT>& os)
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
            if (options.escape_solidus() && c == '/')
            {
                os.put('\\');
                os.put('/');
            }
            else if (is_control_character(c) || options.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                unicons::sequence_generator<const CharT*> g(it,end,unicons::conv_flags::strict);
                if (g.done() || g.status() != unicons::conv_errc())
                {
                    JSONCONS_THROW_EXCEPTION(std::runtime_error,"Invalid codepoint");
                }
                uint32_t cp = g.get().codepoint();
                it += (g.get().length() - 1);
                if (is_non_ascii_codepoint(cp) || is_control_character(c))
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
            else
            {
                os.put(c);
            }
            break;
        }
    }
}

typedef basic_serialization_options<char> serialization_options;
typedef basic_serialization_options<wchar_t> wserialization_options;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_serialization_options<char> output_format;
typedef basic_serialization_options<wchar_t> woutput_format;
#endif

}
#endif
