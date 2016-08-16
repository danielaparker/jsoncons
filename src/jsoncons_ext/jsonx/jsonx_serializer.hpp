// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONX_JSONX_SERIALIZER_HPP
#define JSONCONS_JSONX_JSONX_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <ostream>
#include <cstdlib>
#include <map>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons { namespace jsonx {
 
template <class CharT>
void escape_attribute(const CharT* s,
                      size_t length,
                      const basic_output_format<CharT>& format,
                      buffered_ostream<CharT>& bos)
{
    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '&':
            bos.write("&amp;");
            break;
        case '<':
            bos.write("&lt;");
            break;
        case '\"':
            bos.write("&#34;");
            break;
        default:
            uint32_t u(c >= 0 ? c : 256 + c);
            if (is_control_character(u) || format.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                uint32_t cp = json_text_traits<CharT>::convert_char_to_codepoint(it, end);
                if (is_non_ascii_character(cp) || is_control_character(u))
                {
                    if (cp > 0xFFFF)
                    {
                        cp -= 0x10000;
                        uint32_t first = (cp >> 10) + 0xD800;
                        uint32_t second = ((cp & 0x03FF) + 0xDC00);

                        bos.put('&');
                        bos.put('#');
                        bos.put('x');
                        bos.put(to_hex_character(first >> 12 & 0x000F));
                        bos.put(to_hex_character(first >> 8  & 0x000F));
                        bos.put(to_hex_character(first >> 4  & 0x000F));
                        bos.put(to_hex_character(first     & 0x000F));
                        bos.put(';');
                        bos.put('&');
                        bos.put('#');
                        bos.put('x');
                        bos.put(to_hex_character(second >> 12 & 0x000F));
                        bos.put(to_hex_character(second >> 8  & 0x000F));
                        bos.put(to_hex_character(second >> 4  & 0x000F));
                        bos.put(to_hex_character(second     & 0x000F));
                        bos.put(';');
                    }
                    else
                    {
                        bos.put('\\');
                        bos.put('u');
                        bos.put(to_hex_character(cp >> 12 & 0x000F));
                        bos.put(to_hex_character(cp >> 8  & 0x000F));
                        bos.put(to_hex_character(cp >> 4  & 0x000F));
                        bos.put(to_hex_character(cp     & 0x000F));
                    }
                }
                else
                {
                    bos.put(c);
                }
            }
            else
            {
                bos.put(c);
            }
        }
    }
}
 
template <class CharT>
void escape_value(const CharT* s,
                  size_t length,
                  const basic_output_format<CharT>& format,
                  buffered_ostream<CharT>& bos)
{
    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '&':
            bos.write("&amp;");
            break;
        case '<':
            bos.write("&lt;");
            break;
        case '\"':
            bos.write("&lt;");
            break;
        default:
            uint32_t u(c >= 0 ? c : 256 + c);
            if (is_control_character(u) || format.escape_all_non_ascii())
            {
                // convert utf8 to codepoint
                uint32_t cp = json_text_traits<CharT>::convert_char_to_codepoint(it, end);
                if (is_non_ascii_character(cp) || is_control_character(u))
                {
                    if (cp > 0xFFFF)
                    {
                        cp -= 0x10000;
                        uint32_t first = (cp >> 10) + 0xD800;
                        uint32_t second = ((cp & 0x03FF) + 0xDC00);

                        bos.put('&');
                        bos.put('#');
                        bos.put('x');
                        bos.put(to_hex_character(first >> 12 & 0x000F));
                        bos.put(to_hex_character(first >> 8  & 0x000F));
                        bos.put(to_hex_character(first >> 4  & 0x000F));
                        bos.put(to_hex_character(first     & 0x000F));
                        bos.put(';');
                        bos.put('&');
                        bos.put('#');
                        bos.put('x');
                        bos.put(to_hex_character(second >> 12 & 0x000F));
                        bos.put(to_hex_character(second >> 8  & 0x000F));
                        bos.put(to_hex_character(second >> 4  & 0x000F));
                        bos.put(to_hex_character(second     & 0x000F));
                        bos.put(';');
                    }
                    else
                    {
                        bos.put('\\');
                        bos.put('u');
                        bos.put(to_hex_character(cp >> 12 & 0x000F));
                        bos.put(to_hex_character(cp >> 8  & 0x000F));
                        bos.put(to_hex_character(cp >> 4  & 0x000F));
                        bos.put(to_hex_character(cp     & 0x000F));
                    }
                }
                else
                {
                    bos.put(c);
                }
            }
            else
            {
                bos.put(c);
            }
        }
    }
}

template <class CharT>
struct jsonx_char_traits
{
};

template <>
struct jsonx_char_traits<char>
{
    static const std::string all_literal() {return "all";};

    static const std::string minimal_literal() {return "minimal";};

    static const std::string none_literal() {return "none";};

    static const std::string nonnumeric_literal() {return "nonumeric";};
};

template <>
struct jsonx_char_traits<wchar_t>
{
    static const std::wstring all_literal() {return L"all";};

    static const std::wstring minimal_literal() {return L"minimal";};

    static const std::wstring none_literal() {return L"none";};

    static const std::wstring nonnumeric_literal() {return L"nonumeric";};
};
 
template <class CharT>
void escape_string(const CharT* s,
                   size_t length,
                   CharT quote_char, CharT quote_escape_char,
                   buffered_ostream<CharT>& os)
{
    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        if (c == quote_char)
        {
            os.put(quote_escape_char); 
            os.put(quote_char);
        }
        else
        {
            os.put(c);
        }
    }
}

template<class CharT>
class basic_jsonx_serializer : public basic_json_output_handler<CharT>
{
    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object)
        {
        }
        bool is_object() const
        {
            return is_object_;
        }

        bool is_object_;
        std::basic_string<CharT> name_;
    };
    buffered_ostream<CharT> bos_;
    basic_output_format<CharT> format_;
    std::vector<stack_item> stack_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
    float_printer<CharT> fp_;
    bool indenting_;
    int indent_;
public:
    basic_jsonx_serializer(std::basic_ostream<CharT>& os)
       :
       bos_(os),
       format_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(format_.precision()),
       indenting_(false),
       indent_(0)
    {
    }
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, bool indenting)
       :
       bos_(os),
       format_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(format_.precision()),
       indenting_(indenting),
       indent_(0)
    {
    }
    ~basic_jsonx_serializer()
    {
    }

private:

    void do_begin_json() override
    {
        bos_.write(R"(<?xml version="1.0" encoding="UTF-8"?>)");
        bos_.put('\n');
    }

    void do_end_json() override
    {
    }

    void do_begin_object() override
    {
        if (stack_.size() == 0)
        {
            bos_.write(R"(<json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)");
        }
        else 
        {
            if (indenting_)
            {
                write_indent();
            }
            if (stack_.back().is_object())
            {
                bos_.write("<json:object name=\"");
                escape_attribute(stack_.back().name_.data(),
                                 stack_.back().name_.length(),
                                 format_,
                                 bos_);
                bos_.write("\">");
            }
            else
            {
                bos_.write("<json:object>");
            }
        }
        if (indenting_)
        {
            indent();
        }
        stack_.push_back(stack_item(true));
    }

    void do_end_object() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            write_indent();
        }
        bos_.write("</json:object>");
        stack_.pop_back();
    }

    void do_begin_array() override
    {
        if (stack_.size() == 0)
        {
            bos_.write(R"(<json:array xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)");
            bos_.put('\n');
            stack_.push_back(stack_item(false));
        }
        else 
        {
            if (indenting_)
            {
                write_indent();
            }
            if (stack_.back().is_object())
            {
                bos_.write("<json:array name=\"");
                escape_attribute(stack_.back().name_.data(),
                                 stack_.back().name_.length(),
                                 format_,
                                 bos_);
                bos_.write("\">");
            }
            else
            {
                bos_.write("<json:array>");
            }
            stack_.push_back(stack_item(false));
        }
        if (indenting_)
        {
            indent();
        }
    }

    void do_end_array() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            write_indent();
        }
        bos_.write("</json:array>");
        stack_.pop_back();
    }

    void do_name(const CharT* name, size_t length) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.back().name_ = std::basic_string<CharT>(name,length);
    }

    void do_null_value() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:null name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:number>");
        }
        auto buf = json_text_traits<CharT>::null_literal();
        bos_.write(buf.first,buf.second);
        bos_.write("</json:null>");
    }

    void do_string_value(const CharT* val, size_t length) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:string name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:string>");
        }
        escape_value(val,length,format_,bos_);
        bos_.write("</json:string>");     
    }

    void do_double_value(double value, uint8_t precision) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:number name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:number>");
        }
        if (is_nan(value) && format_.replace_nan())
        {
            bos_.write(format_.nan_replacement());
        }
        else if (is_pos_inf(value) && format_.replace_pos_inf())
        {
            bos_.write(format_.pos_inf_replacement());
        }
        else if (is_neg_inf(value) && format_.replace_neg_inf())
        {
            bos_.write(format_.neg_inf_replacement());
        }
        else
        {
            fp_.print(value,precision,bos_);
        }
        bos_.write("</json:number>");
    }

    void do_integer_value(int64_t value) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:number name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:number>");
        }
        print_integer(value,bos_);
        bos_.write("</json:number>");
    }

    void do_uinteger_value(uint64_t value) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:number name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:number>");
        }
        print_integer(value,bos_);
        bos_.write("</json:number>");
    }

    void do_bool_value(bool value) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            write_indent();
        }
        if (stack_.back().is_object())
        {
            bos_.write("<json:boolean name=\"");
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             format_,
                             bos_);
            bos_.write("\">");
        }
        else
        {
            bos_.write("<json:boolean>");
        }
        if (value)
        {
            auto buf = json_text_traits<CharT>::true_literal();
            bos_.write(buf.first,buf.second);
        }
        else
        {
            auto buf = json_text_traits<CharT>::false_literal();
            bos_.write(buf.first,buf.second);
        }
        bos_.write("</json:boolean>");
    }

    void indent()
    {
        indent_ += static_cast<int>(format_.indent());
    }

    void unindent()
    {
        indent_ -= static_cast<int>(format_.indent());
    }

    void write_indent()
    {
        bos_. put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            bos_. put(' ');
        }
    }
};

typedef basic_jsonx_serializer<char> jsonx_serializer;

}}

#endif
