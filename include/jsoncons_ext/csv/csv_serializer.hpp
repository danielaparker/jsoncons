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
#include <cstdlib>
#include <unordered_map>
#include <memory>
#include <limits> // std::numeric_limits
#include <jsoncons/json_exception.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/detail/number_printers.hpp>
#include <jsoncons/detail/obufferedstream.hpp>
#include <jsoncons_ext/csv/csv_parameters.hpp>
#include <jsoncons/detail/writer.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Writer=jsoncons::detail::ostream_buffered_writer<CharT>,class Allocator=std::allocator<CharT>>
class basic_csv_serializer final : public basic_json_output_handler<CharT>
{
public:
    typedef typename Writer::output_type output_type;

    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef std::basic_string<CharT, std::char_traits<CharT>, char_allocator_type> string_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type> string_allocator_type;

    using typename basic_json_output_handler<CharT>::string_view_type                                 ;
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
    Writer writer_;
    basic_csv_parameters<CharT,Allocator> parameters_;
    basic_serialization_options<CharT> options_;
    std::vector<stack_item> stack_;
    jsoncons::detail::print_double fp_;
    std::vector<string_type,string_allocator_type> column_names_;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::pair<const string_type,string_type>> string_string_allocator_type;
    std::unordered_map<string_type,string_type, std::hash<string_type>,std::equal_to<string_type>,string_string_allocator_type> buffered_line_;

    // Noncopyable and nonmoveable
    basic_csv_serializer(const basic_csv_serializer&) = delete;
    basic_csv_serializer& operator=(const basic_csv_serializer&) = delete;
public:
    basic_csv_serializer(output_type& os)
       :
       writer_(os),
       options_(),
       stack_(),
       fp_(options_.precision()),
       column_names_(parameters_.column_names())
    {
    }

    basic_csv_serializer(output_type& os,
                         const basic_csv_parameters<CharT,Allocator>& params)
       :
       writer_(os),
       parameters_(params),
       options_(),
       stack_(),
       fp_(options_.precision()),
       column_names_(parameters_.column_names())
    {
    }

private:

    template<class AnyWriter>
    void escape_string(const CharT* s,
                       size_t length,
                       CharT quote_char, CharT quote_escape_char,
                       AnyWriter& writer)
    {
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            if (c == quote_char)
            {
                writer.put(quote_escape_char); 
                writer.put(quote_char);
            }
            else
            {
                writer.put(c);
            }
        }
    }

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
        writer_.flush();
    }

    void do_begin_object() override
    {
        stack_.push_back(stack_item(true));
    }

    void do_end_object() override
    {
        if (stack_.size() == 2)
        {
            if (stack_[0].count_ == 0)
            {
                for (size_t i = 0; i < column_names_.size(); ++i)
                {
                    if (i > 0)
                    {
                        writer_.put(parameters_.field_delimiter());
                    }
                    writer_.write(column_names_[i]);
                }
                writer_.write(parameters_.line_delimiter());
            }
            for (size_t i = 0; i < column_names_.size(); ++i)
            {
                if (i > 0)
                {
                    writer_.put(parameters_.field_delimiter());
                }
                auto it = buffered_line_.find(column_names_[i]);
                if (it != buffered_line_.end())
                {
                    writer_.write(it->second);
                    it->second.clear();
                }
            }
            writer_.write(parameters_.line_delimiter());
        }
        stack_.pop_back();

        end_value();
    }

    void do_begin_array() override
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
                        writer_.put(parameters_.field_delimiter());
                    }
                    writer_.write(column_names_[i]);
                }
                if (column_names_.size() > 0)
                {
                    writer_.write(parameters_.line_delimiter());
                }
            }
        }
    }

    void do_end_array() override
    {
        if (stack_.size() == 2)
        {
            writer_.write(parameters_.line_delimiter());
        }
        stack_.pop_back();

        end_value();
    }

    void do_name(const string_view_type& name) override
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
    }

    template <class AnyWriter>
    void write_string(const CharT* s, size_t length, AnyWriter& writer)
    {
        bool quote = false;
        if (parameters_.quote_style() == quote_style_type::all || parameters_.quote_style() == quote_style_type::nonnumeric ||
            (parameters_.quote_style() == quote_style_type::minimal &&
            (std::char_traits<CharT>::find(s, length, parameters_.field_delimiter()) != nullptr || std::char_traits<CharT>::find(s, length, parameters_.quote_char()) != nullptr)))
        {
            quote = true;
            writer.put(parameters_.quote_char());
        }
        escape_string(s, length, parameters_.quote_char(), parameters_.quote_escape_char(), writer);
        if (quote)
        {
            writer.put(parameters_.quote_char());
        }

    }

    void do_null_value() override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    do_null_value(bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                do_null_value(writer_);
            }
        }
    }

    void do_string_value(const string_view_type& val) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,writer_);
            }
        }
    }

    void do_byte_string_value(const uint8_t*, size_t) override
    {

    }

    void do_double_value(double val, const number_format&) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,writer_);
            }
        }
    }

    void do_integer_value(int64_t val) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,writer_);
            }
        }
    }

    void do_uinteger_value(uint64_t val) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,writer_);
            }
        }
    }

    void do_bool_value(bool val) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::detail::string_writer<CharT> bo(s);
                    value(val,bo);
                    bo.flush();
                    it->second = s;
                }
            }
            else
            {
                value(val,writer_);
            }
        }
    }

    template <class AnyWriter>
    void value(const string_view_type& value, AnyWriter& writer)
    {
        begin_value(writer);
        write_string(value.data(),value.length(),writer);
        end_value();
    }

    template <class AnyWriter>
    void value(double val, AnyWriter& writer)
    {
        begin_value(writer);

        if ((std::isnan)(val))
        {
            writer.write(options_.nan_replacement());
        }
        else if (val == std::numeric_limits<double>::infinity())
        {
            writer.write(options_.pos_inf_replacement());
        }
        else if (!(std::isfinite)(val))
        {
            writer.write(options_.neg_inf_replacement());
        }
        else
        {
            fp_(val,options_.precision(),writer);
        }

        end_value();

    }

    template <class AnyWriter>
    void value(int64_t val, AnyWriter& writer)
    {
        begin_value(writer);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        writer.write(ss.str());

        end_value();
    }

    template <class AnyWriter>
    void value(uint64_t val, AnyWriter& writer)
    {
        begin_value(writer);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        writer.write(ss.str());

        end_value();
    }

    template <class AnyWriter>
    void value(bool val, AnyWriter& writer) 
    {
        begin_value(writer);

        if (val)
        {
            auto buf = jsoncons::detail::true_literal<CharT>();
            writer.write(buf,4);
        }
        else
        {
            auto buf = jsoncons::detail::false_literal<CharT>();
            writer.write(buf,5);
        }

        end_value();
    }

    template <class AnyWriter>
    void do_null_value(AnyWriter& writer) 
    {
        begin_value(writer);
        auto buf = jsoncons::detail::null_literal<CharT>();
        writer.write(buf,4);
        end_value();

    }

    template <class AnyWriter>
    void begin_value(AnyWriter& writer)
    {
        if (!stack_.empty())
        {
            if (!stack_.back().is_object_ && stack_.back().count_ > 0)
            {
                writer.put(parameters_.field_delimiter());
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

template <class Json,class Allocator>
void encode_csv(const Json& j, std::basic_ostream<typename Json::char_type>& os, const basic_csv_parameters<typename Json::char_type,Allocator>& params)
{
    typedef typename Json::char_type char_type;
    basic_csv_serializer<char_type,jsoncons::detail::ostream_buffered_writer<char_type>,Allocator> serializer(os,params);
    j.dump(serializer);
}

typedef basic_csv_serializer<char> csv_serializer;

}}

#endif
