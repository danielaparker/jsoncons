// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_SERIALIZER_HPP
#define JSONCONS_JSON_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_out_stream.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons {

template <class Char>
class basic_json_serializer : public basic_json_out_stream<Char>
{
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
    };
public:
    basic_json_serializer(std::basic_ostream<Char>& os)
        : os_(os), indent_(0)
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();
        init();
    }
    basic_json_serializer(std::basic_ostream<Char>& os, basic_output_format<Char> format)
        : os_(os), format_(format), indent_(0)
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();
        init();
    }

    ~basic_json_serializer()
    {
        restore();
    }

    virtual void begin_object()
    {
        begin_value();

        if (format_.indenting() && !stack_.empty() && stack_.back().is_object())
        {
            write_indent();
        }
        stack_.push_back(stack_item(true));
        os_.put('{');
        indent();
    }

    virtual void end_object()
    {
        unindent();
        if (format_.indenting() && !stack_.empty())
        {
            write_indent();
        }
        stack_.pop_back();
        os_.put('}');

        end_element();
    }

    virtual void begin_array()
    {
        begin_value();

        if (format_.indenting() && !stack_.empty() && stack_.back().is_object())
        {
            write_indent();
        }
        stack_.push_back(stack_item(false));
        os_.put('[');
        indent();
    }

    virtual void end_array()
    {
        unindent();
        if (format_.indenting() && !stack_.empty())
        {
            write_indent();
        }
        stack_.pop_back();
        os_.put(']');

        end_element();
    }

    virtual void name(const std::basic_string<Char>& name)
    {
        begin_element();
        os_.put('\"'); 
        escape_string<Char>(name,format_,os_); 
        os_.put('\"'); 
        os_.put(':');
    }

    virtual void value(const std::basic_string<Char>& value)
    {
        begin_value();

        os_.put('\"');
        escape_string<Char>(value,format_,os_);
        os_.put('\"');

        end_element();
    }

    virtual void value(double value)
    {
        begin_value();

        if (is_nan(value) && format_.replace_nan())
        {
            os_  << format_.nan_replacement();
        }
        else if (is_pos_inf(value) && format_.replace_pos_inf())
        {
            os_  << format_.pos_inf_replacement();
        }
        else if (is_neg_inf(value) && format_.replace_neg_inf())
        {
            os_  << format_.neg_inf_replacement();
        }
        else
        {
            os_  << value;
        }

        end_element();
    }

    virtual void value(long long value)
    {
        begin_value();

        os_  << value;

        end_element();
    }

    virtual void value(unsigned long long value)
    {
        begin_value();

        os_  << value;

        end_element();
    }

    virtual void value(bool value)
    {
        begin_value();

        os_ << (value ? json_char_traits<Char>::true_literal() :  json_char_traits<Char>::false_literal());

        end_element();
    }

    virtual void null_value()
    {
        begin_value();

        os_ << json_char_traits<Char>::null_literal();

        end_element();
    }

    virtual void value(const base_userdata<Char>& value)
    {
        begin_value();

        value.to_stream(os_);

        end_element();
    }
protected:

    void begin_element()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                os_.put(',');
            }
            if (format_.indenting())
            {
                write_indent();
            }
        }
    }

    void end_element()
    {
        if (!stack_.empty())
        {
            ++stack_.back().count_;
        }
    }

    void begin_value()
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_element();
        }
    }

    void indent()
    {
        indent_ += format_.indent();
    }

    void unindent()
    {
        indent_ -= format_.indent();
    }

    void write_indent()
    {
        os_.put('\n');
        for (int i = 0; i < indent_; ++i)
        {
            os_.put(' ');
        }
    }

    std::basic_ostream<Char>& os_;
    basic_output_format<Char> format_;
    std::vector<stack_item> stack_;
    int indent_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
private:

    void init()
    {
        os_.setf(format_.set_format_flags());
        os_.unsetf(format_.unset_format_flags());
        os_.precision(format_.precision());
    }

    void restore()
    {
        os_.precision(original_precision_);
        os_.flags(original_format_flags_);
    }
};

typedef basic_json_serializer<char> json_serializer;

typedef basic_output_format<char> output_format;

}
#endif
