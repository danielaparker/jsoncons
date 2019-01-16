// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_SERIALIZER_HPP
#define JSONCONS_CSV_CSV_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <ostream>
#include <utility> // std::move
#include <unordered_map> // std::unordered_map
#include <memory> // std::allocator
#include <limits> // std::numeric_limits
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/print_number.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons/result.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Result=jsoncons::text_stream_result<CharT>,class Allocator=std::allocator<CharT>>
class basic_csv_serializer final : public basic_json_content_handler<CharT>
{
public:
    typedef CharT char_type;
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Result result_type;

    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef std::basic_string<CharT, std::char_traits<CharT>, char_allocator_type> string_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type> string_allocator_type;

private:
    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object), count_(0)
        {
        }
        bool is_object() const
        {
            return is_object_;
        }

        bool is_object_;
        size_t count_;
        string_type name_;
    };
    Result result_;
    basic_csv_options<CharT> parameters_;
    std::vector<stack_item> stack_;
    jsoncons::detail::print_double fp_;
    std::vector<string_type,string_allocator_type> column_names_;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::pair<const string_type,string_type>> string_string_allocator_type;
    std::unordered_map<string_type,string_type, std::hash<string_type>,std::equal_to<string_type>,string_string_allocator_type> buffered_line_;

    // Noncopyable and nonmoveable
    basic_csv_serializer(const basic_csv_serializer&) = delete;
    basic_csv_serializer& operator=(const basic_csv_serializer&) = delete;
public:
    basic_csv_serializer(result_type result)
       : basic_csv_serializer(std::move(result), basic_csv_options<CharT>())
    {
    }

    basic_csv_serializer(result_type result,
                         const basic_csv_options<CharT>& options)
       :
       result_(std::move(result)),
       parameters_(options),
       stack_(),
       fp_(floating_point_options(options.floating_point_format(), 
                                  options.precision(),
                                  0)),
       column_names_(parameters_.column_names())
    {
    }

    ~basic_csv_serializer()
    {
        try
        {
            result_.flush();
        }
        catch (...)
        {
        }
    }

private:

    template<class AnyWriter>
    void escape_string(const CharT* s,
                       size_t length,
                       CharT quote_char, CharT quote_escape_char,
                       AnyWriter& result)
    {
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            if (c == quote_char)
            {
                result.push_back(quote_escape_char); 
                result.push_back(quote_char);
            }
            else
            {
                result.push_back(c);
            }
        }
    }

    void do_flush() override
    {
        result_.flush();
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(true));
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_[0].count_ == 0)
            {
                for (size_t i = 0; i < column_names_.size(); ++i)
                {
                    if (i > 0)
                    {
                        result_.push_back(parameters_.field_delimiter());
                    }
                    result_.insert(column_names_[i].data(),
                                  column_names_[i].length());
                }
                result_.insert(parameters_.line_delimiter().data(),
                              parameters_.line_delimiter().length());
            }
            for (size_t i = 0; i < column_names_.size(); ++i)
            {
                if (i > 0)
                {
                    result_.push_back(parameters_.field_delimiter());
                }
                auto it = buffered_line_.find(column_names_[i]);
                if (it != buffered_line_.end())
                {
                    result_.insert(it->second.data(),it->second.length());
                    it->second.clear();
                }
            }
            result_.insert(parameters_.line_delimiter().data(), parameters_.line_delimiter().length());
        }
        stack_.pop_back();

        end_value();
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(false));
        if (stack_.size() == 2)
        {
            if (stack_[0].count_ == 0)
            {
                for (size_t i = 0; i < column_names_.size(); ++i)
                {
                    if (i > 0)
                    {
                        result_.push_back(parameters_.field_delimiter());
                    }
                    result_.insert(column_names_[i].data(),column_names_[i].length());
                }
                if (column_names_.size() > 0)
                {
                    result_.insert(parameters_.line_delimiter().data(),
                                  parameters_.line_delimiter().length());
                }
            }
        }
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            result_.insert(parameters_.line_delimiter().data(),
                          parameters_.line_delimiter().length());
        }
        stack_.pop_back();

        end_value();
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            stack_.back().name_ = string_type(name);
            buffered_line_[string_type(name)] = std::basic_string<CharT>();
            if (stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                column_names_.push_back(string_type(name));
            }
        }
        return true;
    }

    template <class AnyWriter>
    bool string_value(const CharT* s, size_t length, AnyWriter& result)
    {
        bool quote = false;
        if (parameters_.quote_style() == quote_style_type::all || parameters_.quote_style() == quote_style_type::nonnumeric ||
            (parameters_.quote_style() == quote_style_type::minimal &&
            (std::char_traits<CharT>::find(s, length, parameters_.field_delimiter()) != nullptr || std::char_traits<CharT>::find(s, length, parameters_.quote_char()) != nullptr)))
        {
            quote = true;
            result.push_back(parameters_.quote_char());
        }
        escape_string(s, length, parameters_.quote_char(), parameters_.quote_escape_char(), result);
        if (quote)
        {
            result.push_back(parameters_.quote_char());
        }

        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    accept_null_value(bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                accept_null_value(result_);
            }
        }
        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type, const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    value(sv,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(sv,result_);
            }
        }
        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag_type tag, 
                              const serializing_context& context) override
    {
        byte_string_chars_format encoding_hint;
        switch (tag)
        {
            case semantic_tag_type::base16:
                encoding_hint = byte_string_chars_format::base16;
                break;
            case semantic_tag_type::base64:
                encoding_hint = byte_string_chars_format::base64;
                break;
            case semantic_tag_type::base64url:
                encoding_hint = byte_string_chars_format::base64url;
                break;
            default:
                encoding_hint = byte_string_chars_format::none;
                break;
        }
        byte_string_chars_format format = jsoncons::detail::resolve_byte_string_chars_format(encoding_hint,byte_string_chars_format::none,byte_string_chars_format::base64url);

        std::basic_string<CharT> s;
        switch (format)
        {
            case byte_string_chars_format::base16:
            {
                encode_base16(b.data(),b.length(),s);
                do_string_value(s, semantic_tag_type::none, context);
                break;
            }
            case byte_string_chars_format::base64:
            {
                encode_base64(b.data(),b.length(),s);
                do_string_value(s, semantic_tag_type::none, context);
                break;
            }
            case byte_string_chars_format::base64url:
            {
                encode_base64url(b.data(),b.length(),s);
                do_string_value(s, semantic_tag_type::none, context);
                break;
            }
            default:
            {
                JSONCONS_UNREACHABLE();
            }
        }

        return true;
    }

    bool do_double_value(double val, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    value(val, bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val, result_);
            }
        }
        return true;
    }

    bool do_int64_value(int64_t val, 
                        semantic_tag_type, 
                        const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,result_);
            }
        }
        return true;
    }

    bool do_uint64_value(uint64_t val, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,result_);
            }
        }
        return true;
    }

    bool do_bool_value(bool val, semantic_tag_type, const serializing_context&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_result<std::basic_string<CharT>> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,result_);
            }
        }
        return true;
    }

    template <class AnyWriter>
    void value(const string_view_type& value, AnyWriter& result)
    {
        begin_value(result);
        string_value(value.data(),value.length(),result);
        end_value();
    }

    template <class AnyWriter>
    void value(double val, AnyWriter& result)
    {
        begin_value(result);

        if ((std::isnan)(val))
        {
            result.insert(jsoncons::detail::null_literal<CharT>().data(), jsoncons::detail::null_literal<CharT>().length());
        }
        else if (val == std::numeric_limits<double>::infinity())
        {
            result.insert(jsoncons::detail::null_literal<CharT>().data(), jsoncons::detail::null_literal<CharT>().length());
        }
        else if (!(std::isfinite)(val))
        {
            result.insert(jsoncons::detail::null_literal<CharT>().data(), jsoncons::detail::null_literal<CharT>().length());
        }
        else
        {
            fp_(val, result);
        }

        end_value();

    }

    template <class AnyWriter>
    void value(int64_t val, AnyWriter& result)
    {
        begin_value(result);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        result.insert(ss.str().data(),ss.str().length());

        end_value();
    }

    template <class AnyWriter>
    void value(uint64_t val, AnyWriter& result)
    {
        begin_value(result);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        result.insert(ss.str().data(),ss.str().length());

        end_value();
    }

    template <class AnyWriter>
    void value(bool val, AnyWriter& result) 
    {
        begin_value(result);

        if (val)
        {
            result.insert(jsoncons::detail::true_literal<CharT>().data(),
                         jsoncons::detail::true_literal<CharT>().length());
        }
        else
        {
            result.insert(jsoncons::detail::false_literal<CharT>().data(),
                         jsoncons::detail::false_literal<CharT>().length());
        }

        end_value();
    }

    template <class AnyWriter>
    bool accept_null_value(AnyWriter& result) 
    {
        begin_value(result);
        result.insert(jsoncons::detail::null_literal<CharT>().data(), 
                     jsoncons::detail::null_literal<CharT>().length());
        end_value();
        return true;
    }

    template <class AnyWriter>
    void begin_value(AnyWriter& result)
    {
        if (!stack_.empty())
        {
            if (!stack_.back().is_object_ && stack_.back().count_ > 0)
            {
                result.push_back(parameters_.field_delimiter());
            }
        }
    }

    void end_value()
    {
        if (!stack_.empty())
        {
            ++stack_.back().count_;
        }
    }
};

template <class Json>
void encode_csv(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_csv_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template <class Json>
void encode_csv(const Json& j, std::basic_string<typename Json::char_type>& s)
{
    typedef typename Json::char_type char_type;
    basic_csv_serializer<char_type,jsoncons::string_result<std::basic_string<typename Json::char_type>>> serializer(s);
    j.dump(serializer);
}

template <class Json,class Allocator=std::allocator<char>>
void encode_csv(const Json& j, std::basic_ostream<typename Json::char_type>& os, const basic_csv_options<typename Json::char_type>& options)
{
    typedef typename Json::char_type char_type;
    basic_csv_serializer<char_type,jsoncons::text_stream_result<char_type>,Allocator> serializer(os,options);
    j.dump(serializer);
}

template <class Json,class Allocator=std::allocator<char>>
void encode_csv(const Json& j, std::basic_string<typename Json::char_type>& s, const basic_csv_options<typename Json::char_type>& options)
{
    typedef typename Json::char_type char_type;
    basic_csv_serializer<char_type,jsoncons::string_result<std::basic_string<typename Json::char_type>>,Allocator> serializer(s,options);
    j.dump(serializer);
}

typedef basic_csv_serializer<char> csv_serializer;
typedef basic_csv_serializer<char,jsoncons::string_result<std::string>> csv_string_serializer;

}}

#endif
