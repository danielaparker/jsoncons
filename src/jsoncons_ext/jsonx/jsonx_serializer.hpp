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
#include "jsoncons/json_text_traits.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons { namespace jsonx {

template <class CharT>
struct jsonx_char_traits
{
};

template <>
struct jsonx_char_traits<char>
{
    static const std::string xml_prolog_literal() {return R"(<?xml version="1.0" encoding="UTF-8"?>)";};

    static const std::string top_array_element_literal() {return R"(<json:array xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)";};

    static const std::string top_object_element_literal() {return R"(<json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)";};

    static const std::string array_element_literal() {return "<json:array>";};
    static const std::string object_element_literal() {return "<json:object>";};

    static const std::string object_name_element_literal() {return R"(<json:object name=")";};
    static const std::string array_name_element_literal() {return R"(<json:array name=")";};

    static const std::string close_object_element_literal() {return "</json:object>";};
    static const std::string close_array_element_literal() {return "</json:array>";};

    static const std::string close_tag_literal() {return R"(">)";};

    static const std::string amp_literal() {return "&amp;";}
    static const std::string lt_literal() {return "&lt;";}
    static const std::string quote_literal() {return "&#34;";}
};

template <>
struct jsonx_char_traits<wchar_t>
{
    static const std::wstring xml_prolog_literal() {return LR"(<?xml version="1.0" encoding="UTF-8"?>)";};

    static const std::wstring top_array_element_literal() {return LR"(<json:array xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)";};

    static const std::wstring top_object_element_literal() {return LR"(<json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">)";};

    static const std::wstring array_element_literal() {return L"<json:array>";};
    static const std::wstring object_element_literal() {return L"<json:object>";};

    static const std::wstring object_name_element_literal() {return LR"(<json:object name=")";};
    static const std::wstring array_name_element_literal() {return LR"(<json:array name=")";};

    static const std::wstring close_object_element_literal() {return L"</json:object>";};
    static const std::wstring close_array_element_literal() {return L"</json:array>";};

    static const std::wstring close_tag_literal() {return LR"(">)";};

    static const std::wstring amp_literal() {return L"&amp;";};
    static const std::wstring lt_literal() {return L"&lt;";};
    static const std::wstring quote_literal() {return L"&#34;";};
};
 
template <class CharT>
void escape_attribute(const CharT* s,
                      size_t length,
                      const basic_output_format<CharT>& format,
                      buffered_ostream<CharT>& bos)
{
    (void)format;

    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '&':
            bos.write(jsonx_char_traits<CharT>::amp_literal().data(),
                      jsonx_char_traits<CharT>::amp_literal().length());
            break;
        case '<':
            bos.write(jsonx_char_traits<CharT>::lt_literal().data(),
                      jsonx_char_traits<CharT>::lt_literal().length());
            break;
        case '\"':
            bos.write(jsonx_char_traits<CharT>::quote_literal().data(),
                      jsonx_char_traits<CharT>::quote_literal().length());
            break;
        default:
            // convert utf8 to codepoint
            const CharT* stop = nullptr;
            uint32_t cp = json_text_traits<CharT>::char_sequence_to_codepoint(it, end, &stop);
            if (it == stop)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Invalid codepoint");
            }
            it = stop - 1;
            if (json_text_traits<CharT>::is_non_ascii_codepoint(cp) || json_text_traits<CharT>::is_control_character(c))
            {
                if (cp > 0xFFFF)
                {
                    cp -= 0x10000;
                    uint32_t first = (cp >> 10) + 0xD800;
                    uint32_t second = ((cp & 0x03FF) + 0xDC00);

                    bos.put('&');
                    bos.put('#');
                    bos.put('x');
                    bos.put(json_text_traits<CharT>::to_hex_character(first >> 12 & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(first >> 8  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(first >> 4  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(first     & 0x000F));
                    bos.put(';');
                    bos.put('&');
                    bos.put('#');
                    bos.put('x');
                    bos.put(json_text_traits<CharT>::to_hex_character(second >> 12 & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(second >> 8  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(second >> 4  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(second     & 0x000F));
                    bos.put(';');
                }
                else
                {
                    bos.put('&');
                    bos.put('#');
                    bos.put('x');
                    bos.put(json_text_traits<CharT>::to_hex_character(cp >> 12 & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(cp >> 8  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(cp >> 4  & 0x000F));
                    bos.put(json_text_traits<CharT>::to_hex_character(cp     & 0x000F));
                    bos.put(';');
                }
            }
            else
            {
                bos.put(c);
            }
            break;
        }
    }
}
 
template <class CharT>
void escape_value(const CharT* s,
                  size_t length,
                  const basic_output_format<CharT>& format,
                  buffered_ostream<CharT>& bos)
{
    (void)format;

    const CharT* begin = s;
    const CharT* end = s + length;
    for (const CharT* it = begin; it != end; ++it)
    {
        CharT c = *it;
        switch (c)
        {
        case '&':
            bos.write(jsonx_char_traits<CharT>::amp_literal().data(),
                      jsonx_char_traits<CharT>::amp_literal().length());
            break;
        case '<':
            bos.write(jsonx_char_traits<CharT>::lt_literal().data(),
                      jsonx_char_traits<CharT>::lt_literal().length());
            break;
        case '\"':
            bos.write(jsonx_char_traits<CharT>::quote_literal().data(),
                      jsonx_char_traits<CharT>::quote_literal().length());
            break;
        default:
            bos.put(c);
            break;
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
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, const basic_output_format<CharT>& format)
       :
       bos_(os),
       format_(format),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(format.precision()),
       indenting_(false),
       indent_(0)
    {
    }
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, const basic_output_format<CharT>& format, bool indenting)
       :
       bos_(os),
       format_(format),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(format.precision()),
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
        bos_.write(jsonx_char_traits<CharT>::xml_prolog_literal().data(),
                   jsonx_char_traits<CharT>::xml_prolog_literal().length());
        if (indenting_)
        {
            write_indent();
        }
    }

    void do_end_json() override
    {
        if (indenting_)
        {
            write_indent();
        }
    }

    void do_begin_object() override
    {
        if (stack_.size() == 0)
        {
            bos_.write(jsonx_char_traits<CharT>::top_object_element_literal().data(),
                       jsonx_char_traits<CharT>::top_object_element_literal().length());
        }
        else 
        {
            if (indenting_)
            {
                write_indent();
            }
            if (stack_.back().is_object())
            {
                bos_.write(jsonx_char_traits<CharT>::object_name_element_literal().data(),
                           jsonx_char_traits<CharT>::object_name_element_literal().length());
                escape_attribute(stack_.back().name_.data(),
                                 stack_.back().name_.length(),
                                 format_,
                                 bos_);
                bos_.write(jsonx_char_traits<CharT>::close_tag_literal().data(),
                           jsonx_char_traits<CharT>::close_tag_literal().length());
            }
            else
            {
                bos_.write(jsonx_char_traits<CharT>::object_element_literal().data(),
                           jsonx_char_traits<CharT>::object_element_literal().length());
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
        bos_.write(jsonx_char_traits<CharT>::close_object_element_literal().data(),
                   jsonx_char_traits<CharT>::close_object_element_literal().length());
        stack_.pop_back();
    }

    void do_begin_array() override
    {
        if (stack_.size() == 0)
        {
            bos_.write(jsonx_char_traits<CharT>::top_array_element_literal().data(),
                       jsonx_char_traits<CharT>::top_array_element_literal().length());
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
                bos_.write(jsonx_char_traits<CharT>::array_name_element_literal().data(),
                           jsonx_char_traits<CharT>::array_name_element_literal().length());
                escape_attribute(stack_.back().name_.data(),
                                 stack_.back().name_.length(),
                                 format_,
                                 bos_);
                bos_.write(jsonx_char_traits<CharT>::close_tag_literal().data(),
                           jsonx_char_traits<CharT>::close_tag_literal().length());
            }
            else
            {
                bos_.write(jsonx_char_traits<CharT>::array_element_literal().data(),
                           jsonx_char_traits<CharT>::array_element_literal().length());
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
        bos_.write(jsonx_char_traits<CharT>::close_array_element_literal().data(),
                   jsonx_char_traits<CharT>::close_array_element_literal().length());
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
