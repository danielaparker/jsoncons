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
#include <map>
#include <limits> // std::numeric_limits
#include <jsoncons/json_exception.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/detail/osequencestream.hpp>
#include <jsoncons_ext/csv/csv_parameters.hpp>

namespace jsoncons { namespace csv {

template <class CharT>
void escape_string(const CharT* s,
                   size_t length,
                   CharT quote_char, CharT quote_escape_char,
                   buffered_output<CharT>& os)
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
class basic_csv_serializer : public basic_json_output_handler<CharT>
{
public:
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
        std::basic_string<CharT> name_;
    };
    buffered_output<CharT> os_;
    basic_csv_parameters<CharT> parameters_;
    basic_serialization_options<CharT> options_;
    std::vector<stack_item> stack_;
    print_double<CharT> fp_;
    std::vector<std::basic_string<CharT>> column_names_;
    std::map<std::basic_string<CharT>,std::basic_string<CharT>> buffered_line_;

    // Noncopyable and nonmoveable
    basic_csv_serializer(const basic_csv_serializer&) = delete;
    basic_csv_serializer& operator=(const basic_csv_serializer&) = delete;
public:
    basic_csv_serializer(std::basic_ostream<CharT>& os)
       :
       os_(os),
       options_(),
       stack_(),
       fp_(options_.precision()),
       column_names_(parameters_.column_names())
    {
    }

    basic_csv_serializer(std::basic_ostream<CharT>& os,
                         basic_csv_parameters<CharT> params)
       :
       os_(os),
       parameters_(params),
       options_(),
       stack_(),
       fp_(options_.precision()),
       column_names_(parameters_.column_names())
    {
    }

private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
        os_.flush();
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
                        os_.put(parameters_.field_delimiter());
                    }
                    os_.write(column_names_[i]);
                }
                os_.write(parameters_.line_delimiter());
            }
            for (size_t i = 0; i < column_names_.size(); ++i)
            {
                if (i > 0)
                {
                    os_.put(parameters_.field_delimiter());
                }
                auto it = buffered_line_.find(column_names_[i]);
                if (it != buffered_line_.end())
                {
                    os_.write(it->second);
                    it->second.clear();
                }
            }
            os_.write(parameters_.line_delimiter());
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
                        os_.put(parameters_.field_delimiter());
                    }
                    os_.write(column_names_[i]);
                }
                if (column_names_.size() > 0)
                {
                    os_.write(parameters_.line_delimiter());
                }
            }
        }
    }

    void do_end_array() override
    {
        if (stack_.size() == 2)
        {
            os_.write(parameters_.line_delimiter());
        }
        stack_.pop_back();

        end_value();
    }

    void do_name(string_view_type name) override
    {
        if (stack_.size() == 2)
        {
            stack_.back().name_ = name;
            buffered_line_[name] = std::basic_string<CharT>();
            if (stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                column_names_.push_back(name);
            }
        }
    }

    void write_string(const CharT* s, size_t length, buffered_output<CharT>& os)
    {
        bool quote = false;
        if (parameters_.quote_style() == quote_style_type::all || parameters_.quote_style() == quote_style_type::nonnumeric ||
            (parameters_.quote_style() == quote_style_type::minimal &&
            (std::char_traits<CharT>::find(s, length, parameters_.field_delimiter()) != nullptr || std::char_traits<CharT>::find(s, length, parameters_.quote_char()) != nullptr)))
        {
            quote = true;
            os.put(parameters_.quote_char());
        }
        jsoncons::csv::escape_string<CharT>(s, length, parameters_.quote_char(), parameters_.quote_escape_char(), os);
        if (quote)
        {
            os.put(parameters_.quote_char());
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
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,10);
                    do_null_value(bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                do_null_value(os_);
            }
        }
    }

    void do_string_value(string_view_type val) override
    {
        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,1000);
                    value(val,bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                value(val,os_);
            }
        }
    }

    void do_double_value(double val, uint8_t precision) override
    {
        (void)precision;

        if (stack_.size() == 2)
        {
            if (stack_.back().is_object())
            {
                auto it = buffered_line_.find(stack_.back().name_);
                if (it != buffered_line_.end())
                {
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,30);
                    value(val,bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                value(val,os_);
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
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,30);
                    value(val,bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                value(val,os_);
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
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,30);
                    value(val,bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                value(val,os_);
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
                    basic_osequencestream<CharT> ss;
                    buffered_output<CharT> bo(ss,30);
                    value(val,bo);
                    bo.flush();
                    it->second = std::basic_string<CharT>(ss.data(),ss.length());
                }
            }
            else
            {
                value(val,os_);
            }
        }
    }

    void value(string_view_type value, buffered_output<CharT>& os)
    {
        begin_value(os);
        write_string(value.data(),value.length(),os);
        end_value();
    }

    void value(double val, buffered_output<CharT>& os)
    {
        begin_value(os);

        if ((std::isnan)(val))
        {
            os.write(options_.nan_replacement());
        }
        else if (val == std::numeric_limits<double>::infinity())
        {
            os.write(options_.pos_inf_replacement());
        }
        else if (!(std::isfinite)(val))
        {
            os.write(options_.neg_inf_replacement());
        }
        else
        {
            fp_(val,options_.precision(),os);
        }

        end_value();

    }

    void value(int64_t val, buffered_output<CharT>& os)
    {
        begin_value(os);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        os.write(ss.str());

        end_value();
    }

    void value(uint64_t val, buffered_output<CharT>& os)
    {
        begin_value(os);

        std::basic_ostringstream<CharT> ss;
        ss << val;
        os.write(ss.str());

        end_value();
    }

    void value(bool val, buffered_output<CharT>& os) 
    {
        begin_value(os);

        if (val)
        {
            auto buf = jsoncons::detail::true_literal<CharT>();
            os.write(buf,4);
        }
        else
        {
            auto buf = jsoncons::detail::false_literal<CharT>();
            os.write(buf,5);
        }

        end_value();
    }

    void do_null_value(buffered_output<CharT>& os) 
    {
        begin_value(os);
        auto buf = jsoncons::detail::null_literal<CharT>();
        os.write(buf,4);
        end_value();

    }

    void begin_value(buffered_output<CharT>& os)
    {
        if (!stack_.empty())
        {
            if (!stack_.back().is_object_ && stack_.back().count_ > 0)
            {
                os.put(parameters_.field_delimiter());
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

typedef basic_csv_serializer<char> csv_serializer;

}}

#endif
