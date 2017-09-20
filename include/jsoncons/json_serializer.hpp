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
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/jsoncons_utilities.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_output_handler.hpp>

namespace jsoncons {

template<class CharT>
class basic_json_serializer : public basic_json_output_handler<CharT>
{
public:
    using typename basic_json_output_handler<CharT>::string_view_type                                 ;

private:
    static const size_t default_buffer_length = 16384;

    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object), count_(0), split_lines_(line_split_kind::same_line), indent_once_(false), unindent_at_end_(false)
        {
        }
        stack_item(bool is_object, line_split_kind split_lines, bool indent_once = false)
           : is_object_(is_object), count_(0), split_lines_(split_lines), indent_once_(indent_once), unindent_at_end_(false)
        {
        }

        size_t count() const
        {
            return count_;
        }

        bool unindent_at_end() const
        {
            return unindent_at_end_;
        }

        bool is_object() const
        {
            return is_object_;
        }

        bool is_new_line() const
        {
            return split_lines_ != line_split_kind::same_line;
        }

        bool is_multi_line() const
        {
            return split_lines_ == line_split_kind::multi_line;
        }

        bool is_indent_once() const
        {
            return count_ == 0 ? indent_once_ : false;
        }

        bool is_object_;
        size_t count_;
        line_split_kind split_lines_;
        bool indent_once_;
        bool unindent_at_end_;
    };
    basic_serialization_options<CharT> options_;
    std::vector<stack_item> stack_;
    int indent_;
    bool indenting_;
    print_double<CharT> fp_;
    buffered_output<CharT> bos_;

    // Noncopyable and nonmoveable
    basic_json_serializer(const basic_json_serializer&) = delete;
    basic_json_serializer& operator=(const basic_json_serializer&) = delete;
public:
    basic_json_serializer(std::basic_ostream<CharT>& os)
       : indent_(0), 
         indenting_(false),
         fp_(options_.precision()),
         bos_(os)
    {
    }

    basic_json_serializer(std::basic_ostream<CharT>& os, bool pprint)
       : indent_(0), 
         indenting_(pprint),
         fp_(options_.precision()),
         bos_(os)
    {
    }

    basic_json_serializer(std::basic_ostream<CharT>& os, const basic_serialization_options<CharT>& options)
       : options_(options), 
         indent_(0), 
         indenting_(false),  
         fp_(options_.precision()),
         bos_(os)
    {
    }
    basic_json_serializer(std::basic_ostream<CharT>& os, const basic_serialization_options<CharT>& options, bool pprint)
       : options_(options), 
         indent_(0), 
         indenting_(pprint),  
         fp_(options_.precision()),
         bos_(os)
    {
    }

    ~basic_json_serializer()
    {
    }

private:
    // Implementing methods
    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
        bos_.flush();
    }

    void do_begin_object() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count_ > 0)
                {
                    bos_. put(',');
                }
            }
        }

        if (indenting_)
        {
            if (!stack_.empty() && stack_.back().is_object())
            {
                stack_.push_back(stack_item(true,options_.object_object_split_lines(), false));
            }
            else if (!stack_.empty())
            {
                if (options_.array_object_split_lines() != line_split_kind::same_line)
                {
                    stack_.back().unindent_at_end_ = true;
                    stack_.push_back(stack_item(true,options_.array_object_split_lines(), false));
                    write_indent1();
                }
                else
                {
                    stack_.push_back(stack_item(true,options_.array_object_split_lines(), false));
                }
            }
            else 
            {
                stack_.push_back(stack_item(true, line_split_kind::multi_line, false));
            }
            indent();
        }
        else
        {
            stack_.push_back(stack_item(true));
        }
        bos_.put('{');
    }

    void do_end_object() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().unindent_at_end())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        bos_.put('}');

        end_value();
    }


    void do_begin_array() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count_ > 0)
                {
                    bos_. put(',');
                }
            }
        }
        if (indenting_)
        {
            if (!stack_.empty() && stack_.back().is_object())
            {
                bos_.put('[');
                indent();
                if (options_.object_array_split_lines() != line_split_kind::same_line)
                {
                    stack_.push_back(stack_item(false,options_.object_array_split_lines(),true));
                }
                else
                {
                    stack_.push_back(stack_item(false,options_.object_array_split_lines(),false));
                }
            }
            else if (!stack_.empty())
            {
                if (options_.array_array_split_lines() != line_split_kind::same_line)
                {
                    write_indent();
                }
                stack_.push_back(stack_item(false,options_.array_array_split_lines(), false));
                indent();
                bos_.put('[');
            }
            else 
            {
                stack_.push_back(stack_item(false, line_split_kind::multi_line, false));
                indent();
                bos_.put('[');
            }
        }
        else
        {
            stack_.push_back(stack_item(false));
            bos_.put('[');
        }
    }

    void do_end_array() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().unindent_at_end())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        bos_.put(']');
        end_value();
    }

    void do_name(string_view_type name) override
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                bos_. put(',');
            }
            if (indenting_)
            {
                if (stack_.back().is_multi_line())
                {
                    write_indent();
                }
            }
        }

        bos_.put('\"');
        escape_string<CharT>(name.data(), name.length(), options_, bos_);
        bos_.put('\"');
        bos_.put(':');
        if (indenting_)
        {
            bos_.put(' ');
        }
    }

    void do_null_value() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        auto buf = detail::null_literal<CharT>();
        bos_.write(buf, 4);

        end_value();
    }

    void do_string_value(string_view_type value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        bos_. put('\"');
        escape_string<CharT>(value.data(), value.length(), options_, bos_);
        bos_. put('\"');

        end_value();
    }

    void do_double_value(double value, uint8_t precision) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        if ((std::isnan)(value))
        {
            bos_.write(options_.nan_replacement());
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            bos_.write(options_.pos_inf_replacement());
        }
        else if (!(std::isfinite)(value))
        {
            bos_.write(options_.neg_inf_replacement());
        }
        else
        {
            fp_(value,precision,bos_);
        }

        end_value();
    }

    void do_integer_value(int64_t value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }
        print_integer(value,bos_);
        end_value();
    }

    void do_uinteger_value(uint64_t value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }
        print_uinteger(value,bos_);
        end_value();
    }

    void do_bool_value(bool value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        if (value)
        {
            auto buf = detail::true_literal<CharT>();
            bos_.write(buf,4);
        }
        else
        {
            auto buf = detail::false_literal<CharT>();
            bos_.write(buf,5);
        }

        end_value();
    }

    void begin_scalar_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                bos_. put(',');
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
            if (stack_.back().count_ > 0)
            {
                bos_. put(',');
            }
            if (indenting_)
            {
                if (stack_.back().is_new_line())
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
            ++stack_.back().count_;
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
            stack_.back().unindent_at_end_ = true;
        }
        bos_. put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            bos_. put(' ');
        }
    }

    void write_indent1()
    {
        bos_. put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            bos_. put(' ');
        }
    }
};

typedef basic_json_serializer<char> json_serializer;
typedef basic_json_serializer<wchar_t> wjson_serializer;

}
#endif
