// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_SERIALIZER_HPP
#define JSONCONS_JSON_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_char_traits.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons {

template <class Char>
class basic_json_stream_writer
{
    struct stack_item
    {
        stack_item()
            : count_(0)
        {
        }

        size_t count_;
    };
public:
    basic_json_stream_writer(std::basic_ostream<Char>& os)
        : os_(os), indent_(0)
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();
        init();
    }
    basic_json_stream_writer(std::basic_ostream<Char>& os, basic_output_format<Char> format)
        : os_(os), format_(format), indent_(0)
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();
        init();
    }

    ~basic_json_stream_writer()
    {
        restore();
    }

    void begin_member(const Char* name, size_t length)
    {
        if (stack_.back().count_ > 0)
        {
            os_.put(',');
        }
        write_indent();
        os_.put('\"'); 
        escape_string<Char>(name,length,format_,os_); 
        os_.put('\"'); 
        os_.put(':');
    }

    void end_member()
    {
        ++stack_.back().count_;
    }

    void begin_element()
    {
        if (stack_.back().count_ > 0)
        {
            os_.put(',');
        }
        write_indent();
    }

    void end_element()
    {
        ++stack_.back().count_;
    }

    void value(const Char* value, size_t length)
    {
        os_.put('\"');
        escape_string<Char>(value,length,format_,os_);
        os_.put('\"');
    }

    void value(double value)
    {
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
    }

    void value(long long value)
    {
        os_  << value;
    }

    void value(unsigned long long value)
    {
        os_  << value;
    }

    void value(bool value)
    {
        os_ << (value ? json_char_traits<Char>::true_literal() :  json_char_traits<Char>::false_literal());
    }

    void null()
    {
        os_ << json_char_traits<Char>::null_literal();
    }

    void begin_object()
    {
        //std::cout << "begin_object" << std::endl;
        //write_indent();
        stack_.push_back(stack_item());
        os_.put('{');
        indent();
    }

    void end_object()
    {
        deindent();
        write_indent();
        stack_.pop_back();
        os_.put('}');
    }

    void begin_array()
    {
        //write_indent();
        stack_.push_back(stack_item());
        os_.put('[');
        indent();
    }

    void end_array()
    {
        deindent();
        write_indent();
        stack_.pop_back();
        os_.put(']');
    }

    void restore()
    {
        os_.precision(original_precision_);
        os_.flags(original_format_flags_);
    }
private:

    void init()
    {
        os_.setf(format_.set_format_flags());
        os_.unsetf(format_.unset_format_flags());
        os_.precision(format_.precision());
    }

    void indent()
    {
        indent_ += format_.indent();
    }

    void deindent()
    {
        indent_ -= format_.indent();
    }

    void write_indent()
    {
        if (format_.indenting() && stack_.size() > 0)
        {
            os_.put('\n');
            for (int i = 0; i < indent_; ++i)
            {
                os_.put(' ');
            }
        }
    }

    std::basic_ostream<Char>& os_;
    basic_output_format<Char> format_;
    std::vector<stack_item> stack_;
    int indent_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
};

typedef basic_json_stream_writer<char> json_stream_writer;

typedef basic_output_format<char> output_format;

}
#endif
