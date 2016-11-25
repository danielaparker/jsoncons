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
#include <jsoncons/json_text_traits.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_output_handler.hpp>
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
    static const std::string null_element_literal() {return "<json:null>";};
    static const std::string string_element_literal() {return "<json:string>";};
    static const std::string number_element_literal() {return "<json:number>";};
    static const std::string boolean_element_literal() {return "<json:boolean>";};

    static const std::string object_name_element_literal() {return R"(<json:object name=")";};
    static const std::string array_name_element_literal() {return R"(<json:array name=")";};
    static const std::string null_name_element_literal() {return R"(<json:null name=")";};
    static const std::string string_name_element_literal() {return R"(<json:string name=")";};
    static const std::string number_name_element_literal() {return R"(<json:number name=")";};
    static const std::string boolean_name_element_literal() {return R"(<json:boolean name=")";};

    static const std::string end_object_element_literal() {return "</json:object>";};
    static const std::string end_array_element_literal() {return "</json:array>";};
    static const std::string end_null_element_literal() {return "</json:null>";};
    static const std::string end_string_element_literal() {return "</json:string>";};
    static const std::string end_number_element_literal() {return "</json:number>";};
    static const std::string end_boolean_element_literal() {return "</json:boolean>";};

    static const std::string end_tag_literal() {return R"(">)";};

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
    static const std::wstring null_element_literal() {return L"<json:null>";};
    static const std::wstring string_element_literal() {return L"<json:string>";};
    static const std::wstring number_element_literal() {return L"<json:number>";};
    static const std::wstring boolean_element_literal() {return L"<json:boolean>";};

    static const std::wstring object_name_element_literal() {return LR"(<json:object name=")";};
    static const std::wstring array_name_element_literal() {return LR"(<json:array name=")";};
    static const std::wstring null_name_element_literal() {return LR"(<json:null name=")";};
    static const std::wstring string_name_element_literal() {return LR"(<json:string name=")";};
    static const std::wstring number_name_element_literal() {return LR"(<json:number name=")";};
    static const std::wstring boolean_name_element_literal() {return LR"(<json:boolean name=")";};

    static const std::wstring end_object_element_literal() {return L"</json:object>";};
    static const std::wstring end_array_element_literal() {return L"</json:array>";};
    static const std::wstring end_null_element_literal() {return L"</json:null>";};
    static const std::wstring end_string_element_literal() {return L"</json:string>";};
    static const std::wstring end_number_element_literal() {return L"</json:number>";};
    static const std::wstring end_boolean_element_literal() {return L"</json:boolean>";};

    static const std::wstring end_tag_literal() {return LR"(">)";};

    static const std::wstring amp_literal() {return L"&amp;";};
    static const std::wstring lt_literal() {return L"&lt;";};
    static const std::wstring quote_literal() {return L"&#34;";};
};
 
template <class CharT>
void escape_attribute(const CharT* s,
                      size_t length,
                      const basic_serialization_options<CharT>& options,
                      buffered_output<CharT>& bos)
{
    std::basic_ostringstream<CharT> oss;
    buffered_output<CharT> boos(oss);
    jsoncons::escape_string(s,length,options,boos);
    boos.flush();
    std::basic_string<CharT> str = oss.str();

    const CharT* begin = str.data();
    const CharT* end = str.data() + str.length();
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
                  const basic_serialization_options<CharT>& options,
                  buffered_output<CharT>& bos)
{
    std::basic_ostringstream<CharT> oss;
    buffered_output<CharT> boos(oss);
    jsoncons::escape_string(s,length,options,boos);
    boos.flush();
    std::basic_string<CharT> str = oss.str();

    const CharT* begin = str.data();
    const CharT* end = str.data() + str.length();

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
    buffered_output<CharT> bos_;
    basic_serialization_options<CharT> options_;
    std::vector<stack_item> stack_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
    print_double<CharT> fp_;
    bool indenting_;
    int indent_;

    // Noncopyable and nonmoveable
    basic_jsonx_serializer(const basic_jsonx_serializer&) = delete;
    basic_jsonx_serializer& operator=(const basic_jsonx_serializer&) = delete;
public:
    basic_jsonx_serializer(std::basic_ostream<CharT>& os)
       :
       bos_(os),
       options_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(options_.precision()),
       indenting_(false),
       indent_(0)
    {
    }
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, bool indenting)
       :
       bos_(os),
       options_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(options_.precision()),
       indenting_(indenting),
       indent_(0)
    {
    }
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, const basic_serialization_options<CharT>& options)
       :
       bos_(os),
       options_(options),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(options.precision()),
       indenting_(false),
       indent_(0)
    {
    }
    basic_jsonx_serializer(std::basic_ostream<CharT>& os, const basic_serialization_options<CharT>& options, bool indenting)
       :
       bos_(os),
       options_(options),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       fp_(options.precision()),
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
        bos_.flush();
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
                                 options_,
                                 bos_);
                bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                           jsonx_char_traits<CharT>::end_tag_literal().length());
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
        bos_.write(jsonx_char_traits<CharT>::end_object_element_literal().data(),
                   jsonx_char_traits<CharT>::end_object_element_literal().length());
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
                                 options_,
                                 bos_);
                bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                           jsonx_char_traits<CharT>::end_tag_literal().length());
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
        bos_.write(jsonx_char_traits<CharT>::end_array_element_literal().data(),
                   jsonx_char_traits<CharT>::end_array_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::null_name_element_literal().data(),
                       jsonx_char_traits<CharT>::null_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::null_element_literal().data(),
                       jsonx_char_traits<CharT>::null_element_literal().length());
        }
        auto buf = json_text_traits<CharT>::null_literal();
        bos_.write(buf.first,buf.second);
        bos_.write(jsonx_char_traits<CharT>::end_null_element_literal().data(),
                   jsonx_char_traits<CharT>::end_null_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::string_name_element_literal().data(),
                       jsonx_char_traits<CharT>::string_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::string_element_literal().data(),
                       jsonx_char_traits<CharT>::string_element_literal().length());
        }
        escape_value(val,length,options_,bos_);
        bos_.write(jsonx_char_traits<CharT>::end_string_element_literal().data(),
                   jsonx_char_traits<CharT>::end_string_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::number_name_element_literal().data(),
                       jsonx_char_traits<CharT>::number_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::number_element_literal().data(),
                       jsonx_char_traits<CharT>::number_element_literal().length());
        }
        if (is_nan(value) && options_.replace_nan())
        {
            bos_.write(options_.nan_replacement());
        }
        else if (is_pos_inf(value) && options_.replace_pos_inf())
        {
            bos_.write(options_.pos_inf_replacement());
        }
        else if (is_neg_inf(value) && options_.replace_neg_inf())
        {
            bos_.write(options_.neg_inf_replacement());
        }
        else
        {
            fp_(value,precision,bos_);
        }
        bos_.write(jsonx_char_traits<CharT>::end_number_element_literal().data(),
                   jsonx_char_traits<CharT>::end_number_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::number_name_element_literal().data(),
                       jsonx_char_traits<CharT>::number_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::number_element_literal().data(),
                       jsonx_char_traits<CharT>::number_element_literal().length());
        }
        print_integer(value,bos_);
        bos_.write(jsonx_char_traits<CharT>::end_number_element_literal().data(),
                   jsonx_char_traits<CharT>::end_number_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::number_name_element_literal().data(),
                       jsonx_char_traits<CharT>::number_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::number_element_literal().data(),
                       jsonx_char_traits<CharT>::number_element_literal().length());
        }
        print_integer(value,bos_);
        bos_.write(jsonx_char_traits<CharT>::end_number_element_literal().data(),
                   jsonx_char_traits<CharT>::end_number_element_literal().length());
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
            bos_.write(jsonx_char_traits<CharT>::boolean_name_element_literal().data(),
                       jsonx_char_traits<CharT>::boolean_name_element_literal().length());
            escape_attribute(stack_.back().name_.data(),
                             stack_.back().name_.length(),
                             options_,
                             bos_);
            bos_.write(jsonx_char_traits<CharT>::end_tag_literal().data(),
                       jsonx_char_traits<CharT>::end_tag_literal().length());
        }
        else
        {
            bos_.write(jsonx_char_traits<CharT>::boolean_element_literal().data(),
                       jsonx_char_traits<CharT>::boolean_element_literal().length());
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
        bos_.write(jsonx_char_traits<CharT>::end_boolean_element_literal().data(),
                   jsonx_char_traits<CharT>::end_boolean_element_literal().length());
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
