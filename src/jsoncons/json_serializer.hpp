// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_SERIALIZER_HPP
#define JSONCONS_JSON_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons {

template<typename Char>
class basic_json_serializer : public basic_json_output_handler<Char>
{
    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object), count_(0), content_indented_(false)
        {
        }
        bool is_object() const
        {
            return is_object_;
        }

        bool is_object_;
        size_t count_;
        bool content_indented_;
    };
public:
    basic_json_serializer(std::basic_ostream<Char>& os)
       : os_(os), indent_(0), indenting_(false)
    {
    }

    basic_json_serializer(std::basic_ostream<Char>& os, bool indenting)
       : os_(os), indent_(0), indenting_(indenting)
    {
    }

    basic_json_serializer(std::basic_ostream<Char>& os, const basic_output_format<Char>& format)
       : os_(os), format_(format), indent_(0),
         indenting_(false) // Deprecated behavior
    {
    }
    basic_json_serializer(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting)
       : os_(os), format_(format), indent_(0), indenting_(indenting)
    {
    }

    ~basic_json_serializer()
    {
    }

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object()
    {
        begin_structure();

        if (indenting_ && !stack_.empty() && stack_.back().is_object())
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
        if (indenting_ && !stack_.empty())
        {
            write_indent();
        }
        stack_.pop_back();
        os_.put('}');

        end_value();
    }

    virtual void begin_array()
    {
        begin_structure();

        if (indenting_ && !stack_.empty() && stack_.back().is_object())
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
        if (indenting_ && !stack_.empty() && stack_.back().content_indented_)
        {
            write_indent();
        }
        stack_.pop_back();
        os_.put(']');

        end_value();
    }

    virtual void name(const std::basic_string<Char>& name)
    {
        begin_element();
        os_.put('\"');
        escape_string<Char>(name, format_, os_);
        os_.put('\"');
        os_.put(':');
    }

    virtual void null_value()
    {
        begin_value();

        os_ << json_char_traits<Char,sizeof(Char)>::null_literal();

        end_value();
    }

// value(...) implementation

    virtual void string_value(const std::basic_string<Char>& value)
    {
        begin_value();

        os_.put('\"');
        escape_string<Char>(value, format_, os_);
        os_.put('\"');

        end_value();
    }

    virtual void double_value(double value)
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
        else if (format_.floatfield() != 0)
        {
            std::basic_ostringstream<Char> os;
            os.imbue(std::locale::classic());
            os.setf(format_.floatfield(), std::ios::floatfield);
            os << std::showpoint << std::setprecision(format_.precision()) << value;
            os_ << os.str();
        }
        else
        {
            std::basic_string<Char> buf = double_to_string<Char>(value,format_.precision());
            os_ << buf;
        }

        end_value();
    }

    virtual void longlong_value(long long value)
    {
        begin_value();

        os_  << value;

        end_value();
    }

    virtual void ulonglong_value(unsigned long long value)
    {
        begin_value();

        os_  << value;

        end_value();
    }

    virtual void bool_value(bool value)
    {
        begin_value();

        os_ << (value ? json_char_traits<Char,sizeof(Char)>::true_literal() :  json_char_traits<Char,sizeof(Char)>::false_literal());

        end_value();
    }

    void begin_element()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                os_.put(',');
            }
            if (indenting_)
            {
                write_indent();
            }
        }
    }

    void begin_value()
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            //begin_element();
            if (stack_.back().count_ > 0)
            {
                os_.put(',');
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

    void begin_structure()
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
        if (!stack_.empty())
        {
            stack_.back().content_indented_ = true;
        }
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

    bool indenting_;
};

typedef basic_json_serializer<char> json_serializer;
typedef basic_json_serializer<wchar_t> wjson_serializer;

}
#endif
