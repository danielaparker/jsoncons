// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_SERIALIZER_HPP
#define JSONCONS_JSON_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits> // std::numeric_limits
#include <fstream>
#include <memory>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/byte_string.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons/json_serializing_options.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/print_number.hpp>

namespace jsoncons { namespace detail {
template <class CharT, class Writer>
size_t escape_string(const CharT* s, size_t length,
                     bool escape_all_non_ascii, bool escape_solidus,
                     Writer& writer)
{
    size_t count = 0;
    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '\\':
            writer.push_back('\\');
            writer.push_back('\\');
            count += 2;
            break;
        case '"':
            writer.push_back('\\');
            writer.push_back('\"');
            count += 2;
            break;
        case '\b':
            writer.push_back('\\');
            writer.push_back('b');
            count += 2;
            break;
        case '\f':
            writer.push_back('\\');
            writer.push_back('f');
            count += 2;
            break;
        case '\n':
            writer.push_back('\\');
            writer.push_back('n');
            count += 2;
            break;
        case '\r':
            writer.push_back('\\');
            writer.push_back('r');
            count += 2;
            break;
        case '\t':
            writer.push_back('\\');
            writer.push_back('t');
            count += 2;
            break;
        default:
            if (escape_solidus && c == '/')
            {
                writer.push_back('\\');
                writer.push_back('/');
                count += 2;
            }
            else if (is_control_character(c) || escape_all_non_ascii)
            {
                // convert utf8 to codepoint
                unicons::sequence_generator<const CharT*> g(it, end, unicons::conv_flags::strict);
                if (g.done() || g.status() != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Invalid codepoint"));
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

                        writer.push_back('\\');
                        writer.push_back('u');
                        writer.push_back(to_hex_character(first >> 12 & 0x000F));
                        writer.push_back(to_hex_character(first >> 8 & 0x000F));
                        writer.push_back(to_hex_character(first >> 4 & 0x000F));
                        writer.push_back(to_hex_character(first & 0x000F));
                        writer.push_back('\\');
                        writer.push_back('u');
                        writer.push_back(to_hex_character(second >> 12 & 0x000F));
                        writer.push_back(to_hex_character(second >> 8 & 0x000F));
                        writer.push_back(to_hex_character(second >> 4 & 0x000F));
                        writer.push_back(to_hex_character(second & 0x000F));
                        count += 12;
                    }
                    else
                    {
                        writer.push_back('\\');
                        writer.push_back('u');
                        writer.push_back(to_hex_character(cp >> 12 & 0x000F));
                        writer.push_back(to_hex_character(cp >> 8 & 0x000F));
                        writer.push_back(to_hex_character(cp >> 4 & 0x000F));
                        writer.push_back(to_hex_character(cp & 0x000F));
                        count += 6;
                    }
                }
                else
                {
                    writer.push_back(c);
                    ++count;
                }
            }
            else
            {
                writer.push_back(c);
                ++count;
            }
            break;
        }
    }
    return count;
}
}}

namespace jsoncons {

template<class CharT,class Writer=detail::stream_char_writer<CharT>>
class basic_json_serializer final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;
    typedef typename basic_json_serializing_options<CharT>::string_type string_type;

private:
    enum class structure_type {object, array};

    class serialization_context
    {
        structure_type type_;
        size_t count_;
        line_split_kind line_splits_;
        bool indent_before_;
        bool unindent_after_;
        size_t begin_pos_;
        size_t data_pos_;
    public:
        serialization_context(structure_type type, line_split_kind split_lines, bool indent_once,
                              size_t begin_pos, size_t data_pos)
           : type_(type), count_(0), line_splits_(split_lines), indent_before_(indent_once), unindent_after_(false),
             begin_pos_(begin_pos), data_pos_(data_pos)
        {
        }

        void set_position(size_t pos)
        {
            data_pos_ = pos;
        }

        size_t begin_pos() const
        {
            return begin_pos_;
        }

        size_t data_pos() const
        {
            return data_pos_;
        }

        size_t count() const
        {
            return count_;
        }

        void increment_count()
        {
            ++count_;
        }

        bool unindent_after() const
        {
            return unindent_after_;
        }

        void unindent_after(bool value) 
        {
            unindent_after_ = value;
        }

        bool is_object() const
        {
            return type_ == structure_type::object;
        }

        bool is_array() const
        {
            return type_ == structure_type::array;
        }

        bool is_same_line() const
        {
            return line_splits_ == line_split_kind::same_line;
        }

        bool is_new_line() const
        {
            return line_splits_ == line_split_kind::new_line;
        }

        bool is_multi_line() const
        {
            return line_splits_ == line_split_kind::multi_line;
        }

        bool is_indent_once() const
        {
            return count_ == 0 ? indent_before_ : false;
        }

    };

    size_t indent_size_;
    std::basic_string<CharT> nan_to_num_;
    std::basic_string<CharT> inf_to_num_;
    std::basic_string<CharT> neginf_to_num_;
    std::basic_string<CharT> nan_to_str_;
    std::basic_string<CharT> inf_to_str_;
    std::basic_string<CharT> neginf_to_str_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
    byte_string_chars_format byte_string_format_;
    bignum_chars_format bignum_format_;
    line_split_kind object_object_line_splits_;
    line_split_kind object_array_line_splits_;
    line_split_kind array_array_line_splits_;
    line_split_kind array_object_line_splits_;
    detail::print_double fp_;
    size_t line_length_limit_;
    std::basic_string<CharT> new_line_chars_;
    Writer writer_;

    std::vector<serialization_context> stack_;
    int indent_amount_;
    size_t column_;
    std::basic_string<CharT> colon_str_;
    std::basic_string<CharT> comma_str_;
    std::basic_string<CharT> open_object_brace_str_;
    std::basic_string<CharT> close_object_brace_str_;
    std::basic_string<CharT> open_array_bracket_str_;
    std::basic_string<CharT> close_array_bracket_str_;

    // Noncopyable and nonmoveable
    basic_json_serializer(const basic_json_serializer&) = delete;
    basic_json_serializer& operator=(const basic_json_serializer&) = delete;
public:
    basic_json_serializer(output_type& os)
        : basic_json_serializer(os, basic_json_serializing_options<CharT>())
    {
    }

    basic_json_serializer(output_type& os, 
                          const basic_json_write_options<CharT>& options)
       : indent_size_(options.indent_size()),
         nan_to_num_(options.nan_to_num()),
         inf_to_num_(options.inf_to_num()),
         neginf_to_num_(options.neginf_to_num()),
         nan_to_str_(options.nan_to_str()),
         inf_to_str_(options.inf_to_str()),
         neginf_to_str_(options.neginf_to_str()),
         escape_all_non_ascii_(options.escape_all_non_ascii()),
         escape_solidus_(options.escape_solidus()),
         byte_string_format_(options.byte_string_format()),
         bignum_format_(options.bignum_format()),
         object_object_line_splits_(options.object_object_line_splits()),
         object_array_line_splits_(options.object_array_line_splits()),
         array_array_line_splits_(options.array_array_line_splits()),
         array_object_line_splits_(options.array_object_line_splits()),
         fp_(floating_point_options(options.floating_point_format(), 
                                    options.precision(),
                                    0)),
         line_length_limit_(options.line_length_limit()),
         new_line_chars_(options.new_line_chars()),
         writer_(os), 
         indent_amount_(0), 
         column_(0)
    {
        switch (options.spaces_around_colon())
        {
            case spaces_option::space_after:
                colon_str_ = std::basic_string<CharT>({':',' '});
                break;
            case spaces_option::space_before:
                colon_str_ = std::basic_string<CharT>({' ',':'});
                break;
            case spaces_option::space_before_and_after:
                colon_str_ = std::basic_string<CharT>({' ',':',' '});
                break;
            default:
                colon_str_.push_back(':');
                break;
        }
        switch (options.spaces_around_comma())
        {
            case spaces_option::space_after:
                comma_str_ = std::basic_string<CharT>({',',' '});
                break;
            case spaces_option::space_before:
                comma_str_ = std::basic_string<CharT>({' ',','});
                break;
            case spaces_option::space_before_and_after:
                comma_str_ = std::basic_string<CharT>({' ',',',' '});
                break;
            default:
                comma_str_.push_back(',');
                break;
        }
        if (options.pad_inside_object_braces())
        {
            open_object_brace_str_ = std::basic_string<CharT>('{', ' ');
            close_object_brace_str_ = std::basic_string<CharT>(' ', '}');
        }
        else
        {
            open_object_brace_str_.push_back('{');
            close_object_brace_str_.push_back('}');
        }
        if (options.pad_inside_array_brackets())
        {
            open_array_bracket_str_ = std::basic_string<CharT>('[', ' ');
            close_array_bracket_str_ = std::basic_string<CharT>(' ', ']');
        }
        else
        {
            open_array_bracket_str_.push_back('[');
            close_array_bracket_str_.push_back(']');
        }
    }

    ~basic_json_serializer()
    {
        try
        {
            writer_.flush();
        }
        catch (...)
        {
        }
    }

private:
    // Implementing methods
    void do_flush() override
    {
        writer_.flush();
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.insert(comma_str_.data(),comma_str_.length());
            column_ += comma_str_.length();
        }

        if (!stack_.empty()) // object or array
        {
            if (stack_.back().is_object())
            {
                switch (object_object_line_splits_)
                {
                    case line_split_kind::same_line:
                        if (column_ >= line_length_limit_)
                        {
                            break_line();
                        }
                        break;
                    case line_split_kind::new_line:
                        if (column_ >= line_length_limit_)
                        {
                            break_line();
                        }
                        break;
                    default: // multi_line
                        break;
                }
                stack_.emplace_back(structure_type::object,object_object_line_splits_, false,
                                    column_, column_+open_object_brace_str_.length());
            }
            else // array
            {
                switch (array_object_line_splits_)
                {
                    case line_split_kind::same_line:
                        if (column_ >= line_length_limit_)
                        {
                            stack_.back().unindent_after(true);
                            new_line();
                        }
                        break;
                    case line_split_kind::new_line:
                        stack_.back().unindent_after(true);
                        new_line();
                        break;
                    default: // multi_line
                        stack_.back().unindent_after(true);
                        new_line();
                        break;
                }
                stack_.emplace_back(structure_type::object,array_object_line_splits_, false,
                                    column_, column_+open_object_brace_str_.length());
            }
        }
        else 
        {
            stack_.emplace_back(structure_type::object, line_split_kind::multi_line, false,
                                column_, column_+open_object_brace_str_.length());
        }
        indent();
        
        writer_.insert(open_object_brace_str_.data(), open_object_brace_str_.length());
        column_ += open_object_brace_str_.length();
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        unindent();
        if (stack_.back().unindent_after())
        {
            new_line();
        }
        stack_.pop_back();
        writer_.insert(close_object_brace_str_.data(), close_object_brace_str_.length());
        column_ += close_object_brace_str_.length();

        end_value();
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.insert(comma_str_.data(),comma_str_.length());
            column_ += comma_str_.length();
        }
        if (!stack_.empty())
        {
            if (stack_.back().is_object())
            {
                switch (object_array_line_splits_)
                {
                    case line_split_kind::same_line:
                        stack_.emplace_back(structure_type::array,object_array_line_splits_,false,
                                            column_, column_ + open_array_bracket_str_.length());
                        break;
                    case line_split_kind::new_line:
                    {
                        stack_.emplace_back(structure_type::array,object_array_line_splits_,true,
                                            column_, column_+open_array_bracket_str_.length());
                        break;
                    }
                    default: // multi_line
                        stack_.emplace_back(structure_type::array,object_array_line_splits_,true,
                                            column_, column_+open_array_bracket_str_.length());
                        break;
                }
            }
            else // array
            {
                switch (array_array_line_splits_)
                {
                case line_split_kind::same_line:
                    if (stack_.back().is_multi_line())
                    {
                        stack_.back().unindent_after(true);
                        new_line();
                    }
                    stack_.emplace_back(structure_type::array,array_array_line_splits_, false,
                                        column_, column_+open_array_bracket_str_.length());
                    break;
                case line_split_kind::new_line:
                    stack_.back().unindent_after(true);
                    new_line();
                    stack_.emplace_back(structure_type::array,array_array_line_splits_, false,
                                        column_, column_+open_array_bracket_str_.length());
                    break;
                default: // multi_line
                    stack_.back().unindent_after(true);
                    new_line();
                    stack_.emplace_back(structure_type::array,array_array_line_splits_, false,
                                        column_, column_+open_array_bracket_str_.length());
                    //new_line();
                    break;
                }
            }
        }
        else 
        {
            stack_.emplace_back(structure_type::array, line_split_kind::multi_line, false,
                                column_, column_+open_array_bracket_str_.length());
        }
        indent();
        writer_.insert(open_array_bracket_str_.data(), open_array_bracket_str_.length());
        column_ += open_array_bracket_str_.length();
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        unindent();
        if (stack_.back().unindent_after())
        {
            new_line();
        }
        stack_.pop_back();
        writer_.insert(close_array_bracket_str_.data(), close_array_bracket_str_.length());
        column_ += close_array_bracket_str_.length();
        end_value();
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (stack_.back().count() > 0)
        {
            writer_.insert(comma_str_.data(),comma_str_.length());
            column_ += comma_str_.length();
        }

        if (stack_.back().is_multi_line())
        {
            stack_.back().unindent_after(true);
            new_line();
        }
        else if (stack_.back().count() > 0 && column_ >= line_length_limit_)
        {
            //stack_.back().unindent_after(true);
            new_line(stack_.back().data_pos());
        }

        if (stack_.back().count() == 0)
        {
            stack_.back().set_position(column_);
        }
        writer_.push_back('\"');
        size_t length = jsoncons::detail::escape_string(name.data(), name.length(),escape_all_non_ascii_,escape_solidus_,writer_);
        writer_.push_back('\"');
        writer_.insert(colon_str_.data(),colon_str_.length());
        column_ += (length+2+colon_str_.length());
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }

        writer_.insert(detail::null_literal<CharT>().data(), 
                       detail::null_literal<CharT>().size());
        column_ += detail::null_literal<CharT>().size();

        end_value();
        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type tag, const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }

        switch (tag)
        {
            case semantic_tag_type::bignum:
                write_bignum_value(sv);
                break;
            default:
                write_string_value(sv);
                break;
        }

        end_value();
        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag_type,
                              const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }
        switch (byte_string_format_)
        {
            case byte_string_chars_format::base16:
            {
                writer_.push_back('\"');
                size_t length = encode_base16(b.data(),b.length(),writer_);
                writer_.push_back('\"');
                column_ += (length + 2);
                break;
            }
            case byte_string_chars_format::base64url:
            {
                writer_.push_back('\"');
                size_t length = encode_base64url(b.data(),b.length(),writer_);
                writer_.push_back('\"');
                column_ += (length + 2);
                break;
            }
            default:
            {
                writer_.push_back('\"');
                size_t length = encode_base64(b.data(), b.length(), writer_);
                writer_.push_back('\"');
                column_ += (length + 2);
                break;
            }
        }

        end_value();
        return true;
    }

    bool do_double_value(double value, 
                         const floating_point_options& fmt, 
                         semantic_tag_type,
                         const serializing_context& context) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }

        if ((std::isnan)(value))
        {
            if (!nan_to_num_.empty())
            {
                writer_.insert(nan_to_num_.data(), nan_to_num_.length());
                column_ += nan_to_num_.length();
            }
            else if (!nan_to_str_.empty())
            {
                do_string_value(nan_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
                column_ += detail::null_literal<CharT>().length();
            }
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            if (!inf_to_num_.empty())
            {
                writer_.insert(inf_to_num_.data(), inf_to_num_.length());
                column_ += inf_to_num_.length();
            }
            else if (!inf_to_str_.empty())
            {
                do_string_value(inf_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
                column_ += detail::null_literal<CharT>().length();
            }
        }
        else if (!(std::isfinite)(value))
        {
            if (!neginf_to_num_.empty())
            {
                writer_.insert(neginf_to_num_.data(), neginf_to_num_.length());
                column_ += neginf_to_num_.length();
            }
            else if (!neginf_to_str_.empty())
            {
                do_string_value(neginf_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
                column_ += detail::null_literal<CharT>().length();
            }
        }
        else
        {
            size_t length = fp_(value, fmt, writer_);
            column_ += length;
        }

        end_value();
        return true;
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag_type,
                        const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }
        size_t length = detail::print_integer(value, writer_);
        column_ += length;
        end_value();
        return true;
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }
        size_t length = detail::print_uinteger(value, writer_);
        column_ += length;
        end_value();
        return true;
    }

    bool do_bool_value(bool value, semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty()) 
        {
            if (stack_.back().is_array())
            {
                begin_scalar_value();
            }
            if (!stack_.back().is_multi_line() && column_ >= line_length_limit_)
            {
                break_line();
            }
        }

        if (value)
        {
            writer_.insert(detail::true_literal<CharT>().data(), detail::true_literal<CharT>().length());
            column_ += detail::true_literal<CharT>().length();
        }
        else
        {
            writer_.insert(detail::false_literal<CharT>().data(), detail::false_literal<CharT>().length());
            column_ += detail::false_literal<CharT>().length();
        }

        end_value();
        return true;
    }

    void begin_scalar_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count() > 0)
            {
                writer_.insert(comma_str_.data(),comma_str_.length());
                column_ += comma_str_.length();
            }
            if (stack_.back().is_multi_line() || stack_.back().is_indent_once())
            {
                stack_.back().unindent_after(true);
                new_line();
            }
        }
    }

    void write_string_value(const string_view_type& sv)
    {
        writer_.push_back('\"');
        size_t length = jsoncons::detail::escape_string(sv.data(), sv.length(),escape_all_non_ascii_,escape_solidus_,writer_);
        writer_.push_back('\"');
        column_ += (length+2);
    }

    void write_bignum_value(const string_view_type& sv)
    {
        switch (bignum_format_)
        {
            case bignum_chars_format::integer:
            {
                writer_.insert(sv.data(),sv.size());
                column_ += sv.size();
                break;
            }
            case bignum_chars_format::base64:
            {
                bignum n(sv.data(), sv.length());
                int signum;
                std::vector<uint8_t> v;
                n.dump(signum, v);

                writer_.push_back('\"');
                if (signum == -1)
                {
                    writer_.push_back('~');
                    ++column_;
                }
                size_t length = encode_base64(v.data(), v.size(), writer_);
                writer_.push_back('\"');
                column_ += (length+2);
                break;
            }
            case bignum_chars_format::base64url:
            {
                bignum n(sv.data(), sv.length());
                int signum;
                std::vector<uint8_t> v;
                n.dump(signum, v);

                writer_.push_back('\"');
                if (signum == -1)
                {
                    writer_.push_back('~');
                    ++column_;
                }
                size_t length = encode_base64url(v.data(), v.size(), writer_);
                writer_.push_back('\"');
                column_ += (length+2);
                break;
            }
            default:
            {
                writer_.push_back('\"');
                writer_.insert(sv.data(),sv.size());
                writer_.push_back('\"');
                column_ += (sv.size() + 2);
                break;
            }
        }
    }

    void end_value()
    {
        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
    }

    void indent()
    {
        indent_amount_ += static_cast<int>(indent_size_);
    }

    void unindent()
    {
        indent_amount_ -= static_cast<int>(indent_size_);
    }

    void new_line()
    {
        writer_.insert(new_line_chars_.data(),new_line_chars_.length());
        for (int i = 0; i < indent_amount_; ++i)
        {
            writer_.push_back(' ');
        }
        column_ = indent_amount_;
    }

    void new_line(size_t len)
    {
        writer_.insert(new_line_chars_.data(),new_line_chars_.length());
        for (size_t i = 0; i < len; ++i)
        {
            writer_.push_back(' ');
        }
        column_ = len;
    }

    void break_line()
    {
        stack_.back().unindent_after(true);
        new_line();
    }
};

template<class CharT,class Writer=detail::stream_char_writer<CharT>>
class basic_json_compressed_serializer final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;
    typedef typename basic_json_serializing_options<CharT>::string_type string_type;

private:
    enum class structure_type {object, array};

    class serialization_context
    {
        structure_type type_;
        size_t count_;
    public:
        serialization_context(structure_type type)
           : type_(type), count_(0)
        {
        }

        size_t count() const
        {
            return count_;
        }

        void increment_count()
        {
            ++count_;
        }

        bool is_array() const
        {
            return type_ == structure_type::array;
        }
    };

    std::basic_string<CharT> nan_to_num_;
    std::basic_string<CharT> inf_to_num_;
    std::basic_string<CharT> neginf_to_num_;
    std::basic_string<CharT> nan_to_str_;
    std::basic_string<CharT> inf_to_str_;
    std::basic_string<CharT> neginf_to_str_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
    byte_string_chars_format byte_string_format_;
    bignum_chars_format bignum_format_;

    std::vector<serialization_context> stack_;
    detail::print_double fp_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_json_compressed_serializer(const basic_json_compressed_serializer&) = delete;
    basic_json_compressed_serializer& operator=(const basic_json_compressed_serializer&) = delete;
public:
    basic_json_compressed_serializer(output_type& os)
        : basic_json_compressed_serializer(os, basic_json_serializing_options<CharT>())
    {
    }

    basic_json_compressed_serializer(output_type& os, 
                          const basic_json_write_options<CharT>& options)
       : nan_to_num_(options.nan_to_num()),
         inf_to_num_(options.inf_to_num()),
         neginf_to_num_(options.neginf_to_num()),
         nan_to_str_(options.nan_to_str()),
         inf_to_str_(options.inf_to_str()),
         neginf_to_str_(options.neginf_to_str()),
         escape_all_non_ascii_(options.escape_all_non_ascii()),
         escape_solidus_(options.escape_solidus()),
         byte_string_format_(options.byte_string_format()),
         bignum_format_(options.bignum_format()),
         fp_(floating_point_options(options.floating_point_format(), 
                                    options.precision(),
                                    0)),
         writer_(os)
    {
    }

    ~basic_json_compressed_serializer()
    {
        try
        {
            writer_.flush();
        }
        catch (...)
        {
        }
    }


private:
    // Implementing methods
    void do_flush() override
    {
        writer_.flush();
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        stack_.emplace_back(structure_type::object);
        writer_.push_back('{');
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.push_back('}');

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }


    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }
        stack_.emplace_back(structure_type::array);
        writer_.push_back('[');
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.push_back(']');
        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        writer_.push_back('\"');
        jsoncons::detail::escape_string(name.data(), name.length(),escape_all_non_ascii_,escape_solidus_,writer_);
        writer_.push_back('\"');
        writer_.push_back(':');
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        writer_.insert(detail::null_literal<CharT>().data(), 
                      detail::null_literal<CharT>().size());

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    void write_string_value(const string_view_type& sv)
    {
        writer_.push_back('\"');
        jsoncons::detail::escape_string(sv.data(), sv.length(),escape_all_non_ascii_,escape_solidus_,writer_);
        writer_.push_back('\"');
    }

    void write_bignum_value(const string_view_type& sv)
    {
        switch (bignum_format_)
        {
            case bignum_chars_format::integer:
            {
                writer_.insert(sv.data(),sv.size());
                break;
            }
            case bignum_chars_format::base64:
            {
                bignum n(sv.data(), sv.length());
                int signum;
                std::vector<uint8_t> v;
                n.dump(signum, v);

                writer_.push_back('\"');
                if (signum == -1)
                {
                    writer_.push_back('~');
                }
                encode_base64(v.data(), v.size(), writer_);
                writer_.push_back('\"');
                break;
            }
            case bignum_chars_format::base64url:
            {
                bignum n(sv.data(), sv.length());
                int signum;
                std::vector<uint8_t> v;
                n.dump(signum, v);

                writer_.push_back('\"');
                if (signum == -1)
                {
                    writer_.push_back('~');
                }
                encode_base64url(v.data(), v.size(), writer_);
                writer_.push_back('\"');
                break;
            }
            default:
            {
                writer_.push_back('\"');
                writer_.insert(sv.data(),sv.size());
                writer_.push_back('\"');
                break;
            }
        }
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type tag, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        switch (tag)
        {
            case semantic_tag_type::bignum:
                write_bignum_value(sv);
                break;
            default:
                write_string_value(sv);
                break;
        }

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag_type,
                              const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }
        switch (byte_string_format_)
        {
            case byte_string_chars_format::base16:
            {
                writer_.push_back('\"');
                encode_base16(b.data(),b.length(),writer_);
                writer_.push_back('\"');
                break;
            }
            case byte_string_chars_format::base64url:
            {
                writer_.push_back('\"');
                encode_base64url(b.data(),b.length(),writer_);
                writer_.push_back('\"');
                break;
            }
            default:
            {
                writer_.push_back('\"');
                encode_base64(b.data(), b.length(), writer_);
                writer_.push_back('\"');
                break;
            }
        }

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_double_value(double value, 
                         const floating_point_options& fmt, 
                         semantic_tag_type,
                         const serializing_context& context) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        if ((std::isnan)(value))
        {
            if (!nan_to_num_.empty())
            {
                writer_.insert(nan_to_num_.data(), nan_to_num_.length());
            }
            else if (!nan_to_str_.empty())
            {
                do_string_value(nan_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
            }
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            if (!inf_to_num_.empty())
            {
                writer_.insert(inf_to_num_.data(), inf_to_num_.length());
            }
            else if (!inf_to_str_.empty())
            {
                do_string_value(inf_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
            }
        }
        else if (!(std::isfinite)(value))
        {
            if (!neginf_to_num_.empty())
            {
                writer_.insert(neginf_to_num_.data(), neginf_to_num_.length());
            }
            else if (!neginf_to_str_.empty())
            {
                do_string_value(neginf_to_str_, semantic_tag_type::none, context);
            }
            else
            {
                writer_.insert(detail::null_literal<CharT>().data(), detail::null_literal<CharT>().length());
            }
        }
        else
        {
            fp_(value, fmt, writer_);
        }

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag_type,
                        const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }
        detail::print_integer(value, writer_);
        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }
        detail::print_uinteger(value, writer_);
        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }

    bool do_bool_value(bool value, semantic_tag_type, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
        {
            writer_.push_back(',');
        }

        if (value)
        {
            writer_.insert(detail::true_literal<CharT>().data(),
                           detail::true_literal<CharT>().length());
        }
        else
        {
            writer_.insert(detail::false_literal<CharT>().data(),
                           detail::false_literal<CharT>().length());
        }

        if (!stack_.empty())
        {
            stack_.back().increment_count();
        }
        return true;
    }
};

typedef basic_json_serializer<char,detail::stream_char_writer<char>> json_serializer;
typedef basic_json_serializer<wchar_t,detail::stream_char_writer<wchar_t>> wjson_serializer;

typedef basic_json_compressed_serializer<char,detail::stream_char_writer<char>> json_compressed_serializer;
typedef basic_json_compressed_serializer<wchar_t,detail::stream_char_writer<wchar_t>> wjson_compressed_serializer;

typedef basic_json_serializer<char,detail::string_writer<std::string>> json_string_serializer;
typedef basic_json_serializer<wchar_t,detail::string_writer<std::wstring>> wjson_string_serializer;

}
#endif
