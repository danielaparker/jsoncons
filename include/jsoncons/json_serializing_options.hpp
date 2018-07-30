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

#if !defined(JSONCONS_NO_TO_CHARS)
using chars_format = std::chars_format;
#else
enum class chars_format : uint8_t {fixed=1,scientific=2,hex=4,general=fixed|scientific};
#endif

// floating_point_options

class floating_point_options
{
    chars_format format_;
    uint8_t precision_;
    uint8_t decimal_places_;
public:
    floating_point_options()
        : format_(chars_format::general), precision_(0), decimal_places_(0)
    {
    }

    floating_point_options(chars_format format, uint8_t precision, uint8_t decimal_places = 0)
        : format_(format), precision_(precision), decimal_places_(decimal_places)
    {
    }

    explicit floating_point_options(chars_format format)
        : format_(format), precision_(0), decimal_places_(0)
    {
    }

    floating_point_options(const floating_point_options&) = default;
    floating_point_options(floating_point_options&&) = default;
    floating_point_options& operator=(const floating_point_options& e) = default;
    floating_point_options& operator=(floating_point_options&& e) = default;

    uint8_t precision() const
    {
        return precision_;
    }

    uint8_t decimal_places() const
    {
        return decimal_places_;
    }

    chars_format format() const
    {
        return format_;
    }
};

enum class indenting : uint8_t {no_indent = 0, indent = 1};

#if !defined(JSONCONS_NO_DEPRECATED)
enum class block_options {next_line,same_line};
#endif

enum class line_split_kind  : uint8_t {same_line,new_line,multi_line};

enum class bignum_chars_format : uint8_t {integer, base10, base64, base64url
#if !defined(JSONCONS_NO_DEPRECATED)
,string
#endif
};

enum class byte_string_chars_format : uint8_t {base64,base64url};

template <class CharT>
class basic_json_read_options
{
public:
    typedef std::basic_string<CharT> string_type;

    virtual ~basic_json_read_options() = default;

    virtual bool can_read_nan_replacement() const = 0;

    virtual const string_type& nan_replacement() const = 0;

    virtual bool can_read_pos_inf_replacement() const = 0;

    virtual const string_type& pos_inf_replacement() const = 0;

    virtual bool can_read_neg_inf_replacement() const = 0;

    virtual const string_type& neg_inf_replacement() const = 0;

    virtual size_t max_nesting_depth() const = 0;
};

template <class CharT>
class basic_json_write_options
{
public:
    typedef std::basic_string<CharT> string_type;

    virtual ~basic_json_write_options() = default;

    virtual bool can_write_nan_replacement() const = 0;

    virtual const string_type& nan_replacement() const = 0;

    virtual bool can_write_pos_inf_replacement() const = 0;

    virtual const string_type& pos_inf_replacement() const = 0;

    virtual bool can_write_neg_inf_replacement() const = 0;

    virtual const string_type& neg_inf_replacement() const = 0;

    virtual size_t max_nesting_depth() const = 0;

    virtual byte_string_chars_format byte_string_format() const = 0; 

    virtual bignum_chars_format bignum_format() const = 0; 

    virtual line_split_kind object_object_split_lines() const = 0; 

    virtual line_split_kind array_object_split_lines() const = 0; 

    virtual line_split_kind object_array_split_lines() const = 0; 

    virtual line_split_kind array_array_split_lines() const = 0; 

    virtual int indent() const = 0;  

    virtual chars_format floating_point_format() const = 0; 

    virtual uint8_t precision() const = 0; 

    virtual bool escape_all_non_ascii() const = 0; 

    virtual bool escape_solidus() const = 0; 
};

template <class CharT>
class basic_json_serializing_options : public virtual basic_json_read_options<CharT>, 
                                       public virtual basic_json_write_options<CharT>
{
public:
    typedef CharT char_type;
    typedef basic_string_view<CharT> string_view_type;
    typedef std::basic_string<CharT> string_type;
private:
    int indent_;
    chars_format floating_point_format_;
    uint8_t precision_;
    bool can_read_nan_replacement_;
    bool can_read_pos_inf_replacement_;
    bool can_read_neg_inf_replacement_;
    string_type nan_replacement_;
    string_type pos_inf_replacement_;
    string_type neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
    byte_string_chars_format byte_string_format_;
    bignum_chars_format bignum_format_;
    line_split_kind object_object_split_lines_;
    line_split_kind object_array_split_lines_;
    line_split_kind array_array_split_lines_;
    line_split_kind array_object_split_lines_;

    size_t max_nesting_depth_;
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_json_serializing_options()
        : indent_(default_indent),
          floating_point_format_(chars_format::hex),
          precision_(0),
          can_read_nan_replacement_(false),
          can_read_pos_inf_replacement_(false),
          can_read_neg_inf_replacement_(false),
          escape_all_non_ascii_(false),
          escape_solidus_(false),
          byte_string_format_(byte_string_chars_format::base64url),
          bignum_format_(bignum_chars_format::base10),
          object_object_split_lines_(line_split_kind::multi_line),
          object_array_split_lines_(line_split_kind::same_line),
          array_array_split_lines_(line_split_kind::new_line),
          array_object_split_lines_(line_split_kind::multi_line),
          max_nesting_depth_((std::numeric_limits<size_t>::max)())
    {
    }

//  Properties
    byte_string_chars_format byte_string_format() const override {return byte_string_format_;}
    basic_json_serializing_options<CharT>&  byte_string_format(byte_string_chars_format value) {byte_string_format_ = value; return *this;}

    bignum_chars_format bignum_format() const override {return bignum_format_;}
    basic_json_serializing_options<CharT>&  bignum_format(bignum_chars_format value) {bignum_format_ = value; return *this;}

    line_split_kind object_object_split_lines() const override {return object_object_split_lines_;}
    basic_json_serializing_options<CharT>& object_object_split_lines(line_split_kind value) {object_object_split_lines_ = value; return *this;}

    line_split_kind array_object_split_lines() const override {return array_object_split_lines_;}
    basic_json_serializing_options<CharT>& array_object_split_lines(line_split_kind value) {array_object_split_lines_ = value; return *this;}

    line_split_kind object_array_split_lines() const override {return object_array_split_lines_;}
    basic_json_serializing_options<CharT>& object_array_split_lines(line_split_kind value) {object_array_split_lines_ = value; return *this;}

    line_split_kind array_array_split_lines() const override {return array_array_split_lines_;}
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

    int indent() const override
    {
        return indent_;
    }

    basic_json_serializing_options<CharT>& indent(int value)
    {
        indent_ = value;
        return *this;
    }

    chars_format floating_point_format() const override
    {
        return floating_point_format_;
    }

    basic_json_serializing_options<CharT>& floating_point_format(chars_format value)
    {
        floating_point_format_ = value;
        return *this;
    }

    uint8_t precision() const override
    {
        return precision_;
    }

    basic_json_serializing_options<CharT>& precision(uint8_t value)
    {
        precision_ = value;
        return *this;
    }

    bool escape_all_non_ascii() const override
    {
        return escape_all_non_ascii_;
    }

    basic_json_serializing_options<CharT>& escape_all_non_ascii(bool value)
    {
        escape_all_non_ascii_ = value;
        return *this;
    }

    bool escape_solidus() const override
    {
        return escape_solidus_;
    }

    basic_json_serializing_options<CharT>& escape_solidus(bool value)
    {
        escape_solidus_ = value;
        return *this;
    }

    bool can_read_nan_replacement() const override {return can_read_nan_replacement_;}

    bool can_read_pos_inf_replacement() const override {return can_read_pos_inf_replacement_;}

    bool can_read_neg_inf_replacement() const override {return can_read_neg_inf_replacement_;}

    bool can_write_nan_replacement() const override {return !nan_replacement_.empty();}

    bool can_write_pos_inf_replacement() const override {return !pos_inf_replacement_.empty();}

    bool can_write_neg_inf_replacement() const override {return !neg_inf_replacement_.empty();}

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

    const string_type& nan_replacement() const override
    {
        return nan_replacement_;
    }

    basic_json_serializing_options<CharT>& nan_replacement(const string_view_type& value)
    {
        nan_replacement_ = string_type(value);

        can_read_nan_replacement_ = is_string(value);

        return *this;
    }

    const string_type& pos_inf_replacement() const override
    {
        return pos_inf_replacement_;
    }

    basic_json_serializing_options<CharT>& pos_inf_replacement(const string_view_type& value)
    {
        pos_inf_replacement_ = string_type(value);
        can_read_pos_inf_replacement_ = is_string(value);
        return *this;
    }

    const string_type& neg_inf_replacement() const override
    {
        return neg_inf_replacement_;
    }

    basic_json_serializing_options<CharT>& neg_inf_replacement(const string_view_type& value)
    {
        neg_inf_replacement_ = string_type(value);
        can_read_neg_inf_replacement_ = is_string(value);
        return *this;
    }

    size_t max_nesting_depth() const override
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

typedef basic_json_read_options<char> json_read_options;
typedef basic_json_read_options<wchar_t> wjson_read_options;

typedef basic_json_write_options<char> json_write_options;
typedef basic_json_write_options<wchar_t> wjson_write_options;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_json_serializing_options<char> output_format;
typedef basic_json_serializing_options<wchar_t> woutput_format;
typedef basic_json_serializing_options<char> serialization_options;
typedef basic_json_serializing_options<wchar_t> wserialization_options;
#endif

}
#endif
