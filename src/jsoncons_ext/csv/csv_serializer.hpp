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
#include <jsoncons/json_text_traits.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons_ext/csv/csv_parameters.hpp>

namespace jsoncons { namespace csv {

template <class CharT>
struct csv_string_traits
{
};

template <>
struct csv_string_traits<char>
{
    static const std::string all_literal() {return "all";};

    static const std::string minimal_literal() {return "minimal";};

    static const std::string none_literal() {return "none";};

    static const std::string nonnumeric_literal() {return "nonumeric";};
};

template <>
struct csv_string_traits<wchar_t>
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
           : is_object_(is_object), count_(0), skip_(false)
        {
        }
        bool is_object() const
        {
            return is_object_;
        }

        bool is_object_;
        size_t count_;
        bool skip_;
    };
    buffered_output<CharT> os_;
    basic_csv_parameters<CharT> parameters_;
    basic_serialization_options<CharT> format_;
    std::vector<stack_item> stack_;
    std::basic_ostringstream<CharT> header_oss_;
    buffered_output<CharT> header_os_;
    std::map<std::basic_string<CharT>,size_t> column_name_pos_map_;
    print_double<CharT> fp_;

    // Noncopyable and nonmoveable
    basic_csv_serializer(const basic_csv_serializer&) = delete;
    basic_csv_serializer& operator=(const basic_csv_serializer&) = delete;
public:
    basic_csv_serializer(std::basic_ostream<CharT>& os)
       :
       os_(os),
       format_(),
       stack_(),
       header_os_(header_oss_),
       column_name_pos_map_(),
       fp_(format_.precision())
    {
    }

    basic_csv_serializer(std::basic_ostream<CharT>& os,
                         basic_csv_parameters<CharT> params)
       :
       os_(os),
       parameters_(params),
       format_(),
       stack_(),
       header_os_(header_oss_),
       column_name_pos_map_(),
       fp_(format_.precision())
    {
        if (params.column_names().size() > 0)
        {
            size_t count = 0;
            for (const auto& name : params.column_names())
            {
                column_name_pos_map_[name] = count++;
            }
        }
    }

private:

    void do_begin_json() override
    {
        if (parameters_.column_names().size() > 0)
        {
            bool first = true;
            for (const auto& name : parameters_.column_names())
            {
                if (!first)
                {
                    os_.put(parameters_.field_delimiter());
                }
                else
                {
                    first = false;
                }
                write_string(name.data(), name.length(),os_);
            }
            os_.write(parameters_.line_delimiter());
        }
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
            while (stack_.back().count_ < column_name_pos_map_.size())
            {
                os_.put(parameters_.field_delimiter());
                ++stack_.back().count_;
            }
            os_.write(parameters_.line_delimiter());
            if (stack_[0].count_ == 0)
            {
                header_os_.flush();
                os_.write(header_oss_.str());
            }
        }
        stack_.pop_back();

        end_value();
    }

    void do_begin_array() override
    {
        stack_.push_back(stack_item(false));
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
            if (stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                if (stack_.back().count_ > 0)
                {
                    os_.put(parameters_.field_delimiter());
                }
                write_string(name.data(), name.length(), os_);
                column_name_pos_map_[name] = stack_.back().count_;
            }
            else
            {
                auto it = column_name_pos_map_.find(std::basic_string<CharT>(name));
                if (it == column_name_pos_map_.end())
                {
                    stack_.back().skip_ = true;
                }
                else
                {
                    stack_.back().skip_ = false;
                    while (stack_.back().count_ < it->second)
                    {
                        os_.put(parameters_.field_delimiter());
                        ++stack_.back().count_;
                    }
                }
            }
        }
    }

    void write_string(const CharT* s, size_t length, buffered_output<CharT>& os)
    {
        bool quote = false;
        if (parameters_.quote_style() == quote_styles::all || parameters_.quote_style() == quote_styles::nonnumeric ||
            (parameters_.quote_style() == quote_styles::minimal &&
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
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0  && parameters_.column_names().size() == 0)
            {
                do_null_value(header_os_);
            }
            else
            {
                do_null_value(os_);
            }
        }
    }

    void do_string_value(string_view_type val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                value(val,header_os_);
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

        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,os_);
            }
        }
    }

    void do_integer_value(int64_t val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,os_);
            }
        }
    }

    void do_uinteger_value(uint64_t val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,os_);
            }
        }
    }

    void do_bool_value(bool val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0 && parameters_.column_names().size() == 0)
            {
                value(val,header_os_);
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

        if (is_nan(val) && format_.replace_nan())
        {
            os.write(format_.nan_replacement());
        }
        else if (is_pos_inf(val) && format_.replace_pos_inf())
        {
            os.write(format_.pos_inf_replacement());
        }
        else if (is_neg_inf(val) && format_.replace_neg_inf())
        {
            os.write(format_.neg_inf_replacement());
        }
        else
        {
            fp_(val,format_.precision(),os);
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
            auto buf = json_text_traits<CharT>::true_literal();
            os.write(buf.first,buf.second);
        }
        else
        {
            auto buf = json_text_traits<CharT>::false_literal();
            os.write(buf.first,buf.second);
        }

        end_value();
    }

    void do_null_value(buffered_output<CharT>& os) 
    {
        begin_value(os);
        auto buf = json_text_traits<CharT>::null_literal();
        os.write(buf.first,buf.second);
        end_value();

    }

    void begin_value(buffered_output<CharT>& os)
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
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
