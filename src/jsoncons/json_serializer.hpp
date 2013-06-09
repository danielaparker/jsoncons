// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSONSERIALIZER_HPP
#define JSONCONS_JSONSERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/json2.hpp"

namespace jsoncons {

class output_format
{
public:
    static const size_t default_indent = 4;

    output_format()
        : indenting_(false), indent_(default_indent)
    {
    }

    output_format(bool indenting)
        : indenting_(indenting), indent_(default_indent)
    {
    }

    bool indenting() const
    {
        return indenting_;
    }

    size_t indent() const
    {
        return indent_;
    }
private:
    bool indenting_;
    size_t indent_;
};

template <class Char>
class basic_json_serializer
{
struct stack_item
{
    stack_item(bool is_array)
        : is_array_(is_array), count_(0)
    {
    }
    bool is_array_;
    size_t count_;
};
public:
    basic_json_serializer(std::basic_ostream<Char>& os)
        : os_(os), indent_(0)
    {
    }
    basic_json_serializer(std::basic_ostream<Char>& os, output_format format)
        : os_(os), format_(format), indent_(0)
    {
    }

    void key(const std::string& name)
    {
        if (stack_.back().count_ > 0)
        {
            os_ << ',';
        }
        write_indent();
        os_ << '\"' << escape_string<Char>(name) << '\"' << ':';
    }

    void value(const char* value, size_t length)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_  << '\"' << escape_string<Char>(std::basic_string<Char>(value,length)) << '\"';
        ++stack_.back().count_;
    }

    void value(double value)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_  << value;
        ++stack_.back().count_;
    }

    void value(longlong_type value)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_  << value;
        ++stack_.back().count_;
    }

    void value(ulonglong_type value)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_  << value;
        ++stack_.back().count_;
    }

    void value(bool value)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_ << value ? "true" : "false";
        ++stack_.back().count_;
    }

    void value(nullptr_t)
    {
        if (stack_.back().is_array_)
        {
            if (stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            write_indent();
        }
        os_ << "null";
        ++stack_.back().count_;
    }

    void begin_object()
    {
        if (stack_.size() > 0)
        {
            if (stack_.back().is_array_ && stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            ++stack_.back().count_;
            write_indent();
        }
        stack_.push_back(stack_item(false));
        os_ << "{";
        indent();
    }

    void end_object()
    {
        stack_.pop_back();
        deindent();
        write_indent();
        os_ << '}';
    }

    void begin_array()
    {
        if (stack_.size() > 0)
        {
            if (stack_.back().is_array_ && stack_.back().count_ > 0)
            {
                os_ << ',';
            }
            ++stack_.back().count_;
            write_indent();
        }
        stack_.push_back(stack_item(true));
        os_ << "[";
        indent();
    }

    void end_array()
    {
        stack_.pop_back();
        deindent();
        write_indent();
        os_ << "]";
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
        if (format_.indenting())
        {
            os_ << '\n';
            for (int i = 0; i < indent_; ++i)
            {
                os_.put(' ');
            }
        }
    }
private:
    std::basic_ostream<Char>& os_;
    output_format format_;
    std::vector<stack_item> stack_;
    int indent_;
};

typedef basic_json_serializer<char> json_serializer;

}
#endif
