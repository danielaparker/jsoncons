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
#include <jsoncons/json_serializing_options.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/number_printers.hpp>

namespace jsoncons {

template<class CharT,class Writer=detail::ostream_buffered_writer<CharT>>
class basic_json_serializer final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
    static const size_t default_buffer_length = 16384;

    enum class structure_type {object, array};

    class line_split_context
    {
        structure_type type_;
        size_t count_;
        line_split_kind split_lines_;
        bool indent_before_;
        bool unindent_after_;
    public:
        line_split_context(structure_type type)
           : type_(type), count_(0), split_lines_(line_split_kind::same_line), indent_before_(false), unindent_after_(false)
        {
        }
        line_split_context(structure_type type, line_split_kind split_lines, bool indent_once)
           : type_(type), count_(0), split_lines_(split_lines), indent_before_(indent_once), unindent_after_(false)
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
            return split_lines_ == line_split_kind::same_line;
        }

        bool is_new_line() const
        {
            return split_lines_ == line_split_kind::new_line;
        }

        bool is_multi_line() const
        {
            return split_lines_ == line_split_kind::multi_line;
        }

        bool is_indent_once() const
        {
            return count_ == 0 ? indent_before_ : false;
        }

    };
    basic_json_serializing_options<CharT> options_;
    std::vector<line_split_context> stack_;
    int indent_;
    bool indenting_;
    detail::print_double fp_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_json_serializer(const basic_json_serializer&) = delete;
    basic_json_serializer& operator=(const basic_json_serializer&) = delete;
public:
    basic_json_serializer(output_type& os)
       : indent_(0), 
         indenting_(false),
         fp_(options_.precision()),
         writer_(os)
    {
    }

    basic_json_serializer(output_type& os, indenting line_indent)
       : indent_(0), 
         indenting_(line_indent == indenting::indent),
         fp_(options_.precision()),
         writer_(os)
    {
    }

    basic_json_serializer(output_type& os, const basic_json_serializing_options<CharT>& options)
       : options_(options), 
         indent_(0), 
         indenting_(false),  
         fp_(options_.precision()),
         writer_(os)
    {
    }

    basic_json_serializer(output_type& os, 
                          const basic_json_serializing_options<CharT>& options, 
                          indenting line_indent)
       : options_(options), 
         indent_(0), 
         indenting_(line_indent == indenting::indent),  
         fp_(options_.precision()),
         writer_(os)
    {
    }

    ~basic_json_serializer()
    {
    }


#if !defined(JSONCONS_NO_DEPRECATED)

    basic_json_serializer(output_type& os, bool pprint)
       : indent_(0), 
         indenting_(pprint),
         fp_(options_.precision()),
         writer_(os)
    {
    }

    basic_json_serializer(output_type& os, const basic_json_serializing_options<CharT>& options, bool pprint)
       : options_(options), 
         indent_(0), 
         indenting_(pprint),  
         fp_(options_.precision()),
         writer_(os)
    {
    }
#endif

private:
    void escape_string(const CharT* s,
                       size_t length,
                       const basic_json_serializing_options<CharT>& options,
                       writer_type& writer)
    {
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            switch (c)
            {
            case '\\':
                writer.put('\\'); 
                writer.put('\\');
                break;
            case '"':
                writer.put('\\'); 
                writer.put('\"');
                break;
            case '\b':
                writer.put('\\'); 
                writer.put('b');
                break;
            case '\f':
                writer.put('\\');
                writer.put('f');
                break;
            case '\n':
                writer.put('\\');
                writer.put('n');
                break;
            case '\r':
                writer.put('\\');
                writer.put('r');
                break;
            case '\t':
                writer.put('\\');
                writer.put('t');
                break;
            default:
                if (options.escape_solidus() && c == '/')
                {
                    writer.put('\\');
                    writer.put('/');
                }
                else if (is_control_character(c) || options.escape_all_non_ascii())
                {
                    // convert utf8 to codepoint
                    unicons::sequence_generator<const CharT*> g(it,end,unicons::conv_flags::strict);
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

                            writer.put('\\');
                            writer.put('u');
                            writer.put(to_hex_character(first >> 12 & 0x000F));
                            writer.put(to_hex_character(first >> 8  & 0x000F));
                            writer.put(to_hex_character(first >> 4  & 0x000F));
                            writer.put(to_hex_character(first     & 0x000F));
                            writer.put('\\');
                            writer.put('u');
                            writer.put(to_hex_character(second >> 12 & 0x000F));
                            writer.put(to_hex_character(second >> 8  & 0x000F));
                            writer.put(to_hex_character(second >> 4  & 0x000F));
                            writer.put(to_hex_character(second     & 0x000F));
                        }
                        else
                        {
                            writer.put('\\');
                            writer.put('u');
                            writer.put(to_hex_character(cp >> 12 & 0x000F));
                            writer.put(to_hex_character(cp >> 8  & 0x000F));
                            writer.put(to_hex_character(cp >> 4  & 0x000F));
                            writer.put(to_hex_character(cp     & 0x000F));
                        }
                    }
                    else
                    {
                        writer.put(c);
                    }
                }
                else
                {
                    writer.put(c);
                }
                break;
            }
        }
    }
    // Implementing methods
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
        writer_.flush();
    }

    void do_begin_object(const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count() > 0)
                {
                    writer_. put(',');
                }
            }
        }

        if (indenting_)
        {
            if (!stack_.empty() && stack_.back().is_object())
            {
                stack_.push_back(line_split_context(structure_type::object,options_.object_object_split_lines(), false));
            }
            else if (!stack_.empty())
            {
                if (options_.array_object_split_lines() != line_split_kind::same_line)
                {
                    stack_.back().unindent_after(true);
                    stack_.push_back(line_split_context(structure_type::object,options_.array_object_split_lines(), false));
                    write_indent1();
                }
                else
                {
                    stack_.push_back(line_split_context(structure_type::object,options_.array_object_split_lines(), false));
                }
            }
            else 
            {
                stack_.push_back(line_split_context(structure_type::object, line_split_kind::multi_line, false));
            }
            indent();
        }
        else
        {
            stack_.push_back(line_split_context(structure_type::object));
        }
        writer_.put('{');
    }

    void do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().unindent_after())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        writer_.put('}');

        end_value();
    }


    void do_begin_array(const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count() > 0)
                {
                    writer_. put(',');
                }
            }
        }
        if (indenting_)
        {
            if (!stack_.empty())
            {
                if (stack_.back().is_object())
                {
                    writer_.put('[');
                    indent();
                    if (options_.object_array_split_lines() != line_split_kind::same_line)
                    {
                        stack_.push_back(line_split_context(structure_type::array,options_.object_array_split_lines(),true));
                    }
                    else
                    {
                        stack_.push_back(line_split_context(structure_type::array,options_.object_array_split_lines(),false));
                    }
                }
                else // array
                {
                    if (options_.array_array_split_lines() == line_split_kind::same_line)
                    {
                        if (stack_.back().is_multi_line())
                        {
                            write_indent();
                        }
                        stack_.push_back(line_split_context(structure_type::array,line_split_kind::same_line, false));
                        indent();
                    }
                    else if (options_.array_array_split_lines() == line_split_kind::multi_line)
                    {
                        write_indent();
                        stack_.push_back(line_split_context(structure_type::array,options_.array_array_split_lines(), false));
                        indent();
                    }
                    else // new_line
                    {
                        write_indent();
                        stack_.push_back(line_split_context(structure_type::array,options_.array_array_split_lines(), false));
                        indent();
                    }
                    writer_.put('[');
                }
            }
            else 
            {
                stack_.push_back(line_split_context(structure_type::array, line_split_kind::multi_line, false));
                writer_.put('[');
                indent();
            }
        }
        else
        {
            stack_.push_back(line_split_context(structure_type::array));
            writer_.put('[');
        }
    }

    void do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().unindent_after())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        writer_.put(']');
        end_value();
    }

    void do_name(const string_view_type& name, const serializing_context&) override
    {
        if (!stack_.empty())
        {
            if (stack_.back().count() > 0)
            {
                writer_. put(',');
            }
            if (indenting_)
            {
                if (stack_.back().is_multi_line())
                {
                    write_indent();
                }
            }
        }

        writer_.put('\"');
        escape_string(name.data(), name.length(), options_, writer_);
        writer_.put('\"');
        writer_.put(':');
        if (indenting_)
        {
            writer_.put(' ');
        }
    }

    void do_null_value(const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }

        auto buf = detail::null_literal<CharT>();
        writer_.write(buf, 4);

        end_value();
    }

    void do_string_value(const string_view_type& value, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }

        writer_. put('\"');
        escape_string(value.data(), value.length(), options_, writer_);
        writer_. put('\"');

        end_value();
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) override
    {
        std::basic_string<CharT> s;
        encode_base64url(data,data+length,s);
        do_string_value(s, context);
    }

    void do_double_value(double value, const number_format& fmt, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }

        if ((std::isnan)(value))
        {
            if (options_.can_write_nan_replacement())
            {
                writer_.write(options_.nan_replacement());
            }
            else
            {
                writer_.write(detail::null_literal<CharT>());
            }
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            if (options_.can_write_pos_inf_replacement())
            {
                writer_.write(options_.pos_inf_replacement());
            }
            else
            {
                writer_.write(detail::null_literal<CharT>());
            }
        }
        else if (!(std::isfinite)(value))
        {
            if (options_.can_write_neg_inf_replacement())
            {
                writer_.write(options_.neg_inf_replacement());
            }
            else
            {
                writer_.write(detail::null_literal<CharT>());
            }
        }
        else
        {
            fp_(value, fmt.precision(), writer_);
        }

        end_value();
    }

    void do_integer_value(int64_t value, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }
        detail::print_integer(value, writer_);
        end_value();
    }

    void do_uinteger_value(uint64_t value, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }
        detail::print_uinteger(value, writer_);
        end_value();
    }

    void do_bool_value(bool value, const serializing_context&) override
    {
        if (!stack_.empty() && stack_.back().is_array())
        {
            begin_scalar_value();
        }

        if (value)
        {
            auto buf = detail::true_literal<CharT>();
            writer_.write(buf,4);
        }
        else
        {
            auto buf = detail::false_literal<CharT>();
            writer_.write(buf,5);
        }

        end_value();
    }

    void begin_scalar_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count() > 0)
            {
                writer_. put(',');
            }
            if (indenting_)
            {
                if (stack_.back().is_multi_line() || stack_.back().is_indent_once())
                {
                    write_indent();
                }
            }
        }
    }

    void begin_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count() > 0)
            {
                writer_. put(',');
            }
            if (indenting_)
            {
                if (!stack_.back().is_same_line())
                {
                    write_indent();
                }
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
        indent_ += static_cast<int>(options_.indent());
    }

    void unindent()
    {
        indent_ -= static_cast<int>(options_.indent());
    }

    void write_indent()
    {
        if (!stack_.empty())
        {
            stack_.back().unindent_after(true);
        }
        writer_. put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            writer_. put(' ');
        }
    }

    void write_indent1()
    {
        writer_. put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            writer_. put(' ');
        }
    }
};

typedef basic_json_serializer<char,detail::ostream_buffered_writer<char>> json_serializer;
typedef basic_json_serializer<wchar_t,detail::ostream_buffered_writer<wchar_t>> wjson_serializer;

typedef basic_json_serializer<char,detail::string_writer<char>> json_string_serializer;
typedef basic_json_serializer<wchar_t,detail::string_writer<wchar_t>> wjson_string_serializer;

}
#endif
