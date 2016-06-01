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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json_output_handler.hpp"

namespace jsoncons {

template<typename CharT>
class basic_json_serializer : public basic_json_output_handler<CharT>
{
    static const size_t default_buffer_length = 16384;

    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object), count_(0), option_(block_options::next_line), multiline_(false)
        {
            scalar_option_ = is_object ? block_options::next_line : block_options::same_line;
        }
        stack_item(bool is_object, block_options option)
           : is_object_(is_object), count_(0), option_(option), multiline_(false)
        {
            scalar_option_ = is_object ? block_options::next_line : block_options::same_line;
        }
        bool is_multiline() const
        {
            return multiline_;
        }

        bool is_object() const
        {
            return is_object_;
        }

        bool is_same_line() const
        {
            return option_ = block_options::same_line;
        }

        bool is_next_line() const
        {
            return option_ == block_options::next_line;
        }

        bool scalar_next_line() const
        {
            return scalar_option_ == block_options::next_line;
        }

        bool is_object_;
        size_t count_;
        block_options option_;
        block_options scalar_option_;
        bool multiline_;
    };
    basic_output_format<CharT> format_;
    std::vector<stack_item> stack_;
    int indent_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
    bool indenting_;
    float_printer<CharT> fp_;
    buffered_ostream<CharT> bos_;
public:
    basic_json_serializer(std::basic_ostream<CharT>& os)
       : indent_(0), 
         indenting_(false),
         fp_(format_.precision()),
         bos_(os)
    {
    }

    basic_json_serializer(std::basic_ostream<CharT>& os, bool indenting)
       : indent_(0), 
         indenting_(indenting),
         fp_(format_.precision()),
         bos_(os)
    {
    }

    basic_json_serializer(std::basic_ostream<CharT>& os, const basic_output_format<CharT>& format)
       : format_(format), 
         indent_(0),
         indenting_(false),  
         fp_(format_.precision()),
         bos_(os)
    {
    }
    basic_json_serializer(std::basic_ostream<CharT>& os, const basic_output_format<CharT>& format, bool indenting)
       : format_(format), 
         indent_(0), 
         indenting_(indenting),  
         fp_(format_.precision()),
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
                if (format_.object_object_block_option() == block_options::next_line)
                {
                    write_indent();
                }
                stack_.push_back(stack_item(true,format_.object_object_block_option()));
            }
            else if (!stack_.empty())
            {
                if (format_.array_object_block_option() == block_options::next_line)
                {
                    write_indent();
                }
                stack_.push_back(stack_item(true,format_.array_object_block_option()));
            }
            else
            {
                stack_.push_back(stack_item(true));
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
            write_indent();
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
                if (format_.object_array_block_option() == block_options::next_line)
                {
                    write_indent();
                }
                stack_.push_back(stack_item(false,format_.object_array_block_option()));
            }
            else if (!stack_.empty())
            {
                if (format_.array_array_block_option() == block_options::next_line)
                {
                    write_indent();
                }
                stack_.push_back(stack_item(false,format_.array_array_block_option()));
            }
            else
            {
                stack_.push_back(stack_item(false));
            }
            indent();
        }
        else
        {
            stack_.push_back(stack_item(false));
        }
        bos_.put('[');
    }

    void do_end_array() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().is_multiline())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        bos_.put(']');
        end_value();
    }

    void do_name(const CharT* name, size_t length) override
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                bos_. put(',');
            }
            if (indenting_)
            {
                if (stack_.back().scalar_next_line())
                {
                    write_indent();
                }
            }
        }

        bos_.put('\"');
        escape_string<CharT>(name, length, format_, bos_);
        bos_.put('\"');
        bos_.put(':');
        bos_.put(' ');
    }

    void do_null_value() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        auto buf = json_literals<CharT>::null_literal();
        bos_.write(buf.first,buf.second);

        end_value();
    }

    void do_string_value(const CharT* value, size_t length) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        bos_. put('\"');
        escape_string<CharT>(value, length, format_, bos_);
        bos_. put('\"');

        end_value();
    }

    void do_double_value(double value, uint8_t precision) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
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
            auto buf = json_literals<CharT>::true_literal();
            bos_.write(buf.first,buf.second);
        }
        else
        {
            auto buf = json_literals<CharT>::false_literal();
            bos_.write(buf.first,buf.second);
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
                if (stack_.back().scalar_next_line())
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
            if (indenting_ && stack_.back().is_next_line())
            {
                write_indent();
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
        indent_ += static_cast<int>(format_.indent());
    }

    void unindent()
    {
        indent_ -= static_cast<int>(format_.indent());
    }

    void write_indent()
    {
        if (!stack_.empty())
        {
            stack_.back().multiline_ = true;
        }
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
