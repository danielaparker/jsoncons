// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_SERIALIZING_OPTIONS_HPP
#define JSONCONS_JSON_SERIALIZING_OPTIONS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>

namespace jsoncons {

enum class indenting {no_indent = 0, indent = 1};

#if !defined(JSONCONS_NO_DEPRECATED)
enum class block_options {next_line,same_line};
#endif

enum class line_split_kind{same_line,new_line,multi_line};

template <class CharT,class Allocator=std::allocator<CharT>>
class basic_json_serializing_options
{
public:
    typedef basic_string_view_ext<CharT> string_view_type;
    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type> string_type;
private:
    int indent_;
    uint8_t precision_;
    uint8_t decimal_places_;
    bool can_read_nan_replacement_;
    bool can_read_pos_inf_replacement_;
    bool can_read_neg_inf_replacement_;
    string_type nan_replacement_;
    string_type pos_inf_replacement_;
    string_type neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;

    line_split_kind object_object_split_lines_;
    line_split_kind object_array_split_lines_;
    line_split_kind array_array_split_lines_;
    line_split_kind array_object_split_lines_;

    chars_format floating_point_format_;
    size_t max_nesting_depth_;
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_json_serializing_options()
        : indent_(default_indent),
          precision_(0),
          can_read_nan_replacement_(false),
          can_read_pos_inf_replacement_(false),
          can_read_neg_inf_replacement_(false),
          escape_all_non_ascii_(false),
          escape_solidus_(false),
          object_object_split_lines_(line_split_kind::multi_line),
          object_array_split_lines_(line_split_kind::same_line),
          array_array_split_lines_(line_split_kind::new_line),
          array_object_split_lines_(line_split_kind::multi_line),
          max_nesting_depth_((std::numeric_limits<size_t>::max)())
    {
    }

//  Accessors
    line_split_kind object_object_split_lines() const {return object_object_split_lines_;}
    line_split_kind array_object_split_lines() const {return array_object_split_lines_;}
    line_split_kind object_array_split_lines() const {return object_array_split_lines_;}
    line_split_kind array_array_split_lines() const {return array_array_split_lines_;}

    basic_json_serializing_options<CharT>& object_object_split_lines(line_split_kind value) {object_object_split_lines_ = value; return *this;}
    basic_json_serializing_options<CharT>& array_object_split_lines(line_split_kind value) {array_object_split_lines_ = value; return *this;}
    basic_json_serializing_options<CharT>& object_array_split_lines(line_split_kind value) {object_array_split_lines_ = value; return *this;}
    basic_json_serializing_options<CharT>& array_array_split_lines(line_split_kind value) {array_array_split_lines_ = value; return *this;}

#if !defined(JSONCONS_NO_DEPRECATED)
    block_options array_array_block_option()
    {
        return (array_array_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_json_serializing_options<CharT>& array_array_block_option(block_options value)
    {
        array_array_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options array_object_block_option()
    {
        return (array_object_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_json_serializing_options<CharT>& array_object_block_option(block_options value)
    {
        array_object_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options object_array_block_option()
    {
        return (object_array_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_json_serializing_options<CharT>& object_array_block_option(block_options value)
    {
        object_array_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }

    block_options object_object_block_option()
    {
        return (object_object_split_lines_ == line_split_kind::same_line) ? block_options::same_line : block_options::next_line;
    }

    basic_json_serializing_options<CharT>& object_object_block_option(block_options value)
    {
        object_object_split_lines_ = (value == block_options::same_line) ? line_split_kind::same_line : line_split_kind::new_line;
        return *this;
    }
#endif

    int indent() const
    {
        return indent_;
    }

    basic_json_serializing_options<CharT>& indent(int value)
    {
        indent_ = value;
        return *this;
    }

    chars_format floating_point_format() const
    {
        return floating_point_format_;
    }

    basic_json_serializing_options<CharT>& floating_point_format(chars_format value)
    {
        floating_point_format_ = value;
        return *this;
    }

    uint8_t precision() const
    {
        return precision_;
    }

    basic_json_serializing_options<CharT>& precision(uint8_t value)
    {
        precision_ = value;
        return *this;
    }

    uint8_t decimal_places() const
    {
        return decimal_places_;
    }

    basic_json_serializing_options<CharT>& decimal_places(uint8_t value)
    {
        decimal_places_ = value;
        return *this;
    }

    bool escape_all_non_ascii() const
    {
        return escape_all_non_ascii_;
    }

    basic_json_serializing_options<CharT>& escape_all_non_ascii(bool value)
    {
        escape_all_non_ascii_ = value;
        return *this;
    }

    bool escape_solidus() const
    {
        return escape_solidus_;
    }

    basic_json_serializing_options<CharT>& escape_solidus(bool value)
    {
        escape_solidus_ = value;
        return *this;
    }

    bool can_read_nan_replacement() const {return can_read_nan_replacement_;}

    bool can_read_pos_inf_replacement() const {return can_read_pos_inf_replacement_;}

    bool can_read_neg_inf_replacement() const {return can_read_neg_inf_replacement_;}

    bool can_write_nan_replacement() const {return !nan_replacement_.empty();}

    bool can_write_pos_inf_replacement() const {return !pos_inf_replacement_.empty();}

    bool can_write_neg_inf_replacement() const {return !neg_inf_replacement_.empty();}

    basic_json_serializing_options<CharT>& replace_inf(bool replace)
    {
        can_read_pos_inf_replacement_ = replace;
        can_read_neg_inf_replacement_ = replace;
        return *this;
    }

    basic_json_serializing_options<CharT>& replace_pos_inf(bool replace)
    {
        can_read_pos_inf_replacement_ = replace;
        return *this;
    }

    basic_json_serializing_options<CharT>& replace_neg_inf(bool replace)
    {
        can_read_neg_inf_replacement_ = replace;
        return *this;
    }

    const string_type& nan_replacement() const
    {
        return nan_replacement_;
    }

    basic_json_serializing_options<CharT>& nan_replacement(const string_type& replacement)
    {
        nan_replacement_ = replacement;

        can_read_nan_replacement_ = is_string(replacement);

        return *this;
    }

    const string_type& pos_inf_replacement() const
    {
        return pos_inf_replacement_;
    }

    basic_json_serializing_options<CharT>& pos_inf_replacement(const string_type& replacement)
    {
        pos_inf_replacement_ = replacement;
        can_read_pos_inf_replacement_ = is_string(replacement);
        return *this;
    }

    const string_type& neg_inf_replacement() const
    {
        return neg_inf_replacement_;
    }

    basic_json_serializing_options<CharT>& neg_inf_replacement(const string_type& replacement)
    {
        neg_inf_replacement_ = replacement;
        can_read_neg_inf_replacement_ = is_string(replacement);
        return *this;
    }

    size_t max_nesting_depth() const
    {
        return max_nesting_depth_;
    }

    void max_nesting_depth(size_t value)
    {
        max_nesting_depth_ = value;
    }
private:
    enum class input_state {initial,begin_quote,character,end_quote,escape,error};
    bool is_string(const string_view_type& s) const
    {
        input_state state = input_state::initial;
        for (CharT c : s)
        {
            switch (c)
            {
            case '\t': case ' ': case '\n': case'\r':
                break;
            case '\\':
                state = input_state::escape;
                break;
            case '\"':
                switch (state)
                {
                case input_state::initial:
                    state = input_state::begin_quote;
                    break;
                case input_state::begin_quote:
                    state = input_state::end_quote;
                    break;
                case input_state::character:
                    state = input_state::end_quote;
                    break;
                case input_state::end_quote:
                    state = input_state::error;
                    break;
                case input_state::escape:
                    state = input_state::character;
                    break;
                default:
                    state = input_state::character;
                    break;
                }
            default:
                break;
            }

        }
        return state == input_state::end_quote;
    }
};

typedef basic_json_serializing_options<char> json_serializing_options;
typedef basic_json_serializing_options<wchar_t> wjson_serializing_options;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_json_serializing_options<char> output_format;
typedef basic_json_serializing_options<wchar_t> woutput_format;
typedef basic_json_serializing_options<char> serialization_options;
typedef basic_json_serializing_options<wchar_t> wserialization_options;
#endif

}
#endif
