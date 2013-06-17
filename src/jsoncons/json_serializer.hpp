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
#include "jsoncons/json_char_traits.hpp"
//#include <math.h> // isnan
#include <limits> // std::numeric_limits

namespace jsoncons {

#ifdef _MSC_VER
inline bool is_nan(double x) { return _isnan( x ) != 0; }
inline bool is_inf(double x)
{
    return !_finite(x) && !_isnan(x);
}
inline bool is_pos_inf(double x)
{
    return is_inf(x) && x > 0;
}
inline bool is_neg_inf(double x)
{
    return is_inf(x) && x < 0;
}
#else
inline bool is_nan(double x) { return std::isnan( x ); }
inline bool is_pos_inf(double x) {return std::isinf() && x > 0;}
inline bool is_neg_inf(double x) {return  std::isinf() && x > 0;}
#endif

template <class Char>
class basic_output_format
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_output_format()
        : indenting_(false), 
          indent_(default_indent),
          precision_(16),
          replace_nan_(true),replace_pos_inf_(true),replace_neg_inf_(true), 
          pos_inf_replacement_(json_char_traits<Char>::null_literal()),
          neg_inf_replacement_(json_char_traits<Char>::null_literal()),
          nan_replacement_(json_char_traits<Char>::null_literal()),
          escape_all_non_ascii_(false),
          escape_solidus_(false)
    {
    }

    basic_output_format(bool indenting)
        : indenting_(indenting), 
          precision_(16),
          indent_(default_indent),
          replace_nan_(true),replace_pos_inf_(true),replace_neg_inf_(true), 
          pos_inf_replacement_(json_char_traits<Char>::null_literal()),
          neg_inf_replacement_(json_char_traits<Char>::null_literal()),
          nan_replacement_(json_char_traits<Char>::null_literal()),
          escape_all_non_ascii_(false),
          escape_solidus_(false)
    {
    }

//  Accessors

    bool indenting() const
    {
        return indenting_;
    }

    size_t indent() const
    {
        return indent_;
    }

    std::streamsize precision() const 
    {
        return precision_; 
    }

    bool escape_all_non_ascii() const
    {
        return escape_all_non_ascii_;
    }

    bool escape_solidus() const
    {
        return escape_solidus_;
    }

    bool replace_nan() const {return replace_nan_;}

    bool replace_pos_inf() const {return replace_pos_inf_;}

    bool replace_neg_inf() const {return replace_neg_inf_;}

    std::basic_string<Char> nan_replacement() const 
    {
        return nan_replacement_;
    }

    std::basic_string<Char> pos_inf_replacement() const 
    {
        return pos_inf_replacement_;
    }

    std::basic_string<Char> neg_inf_replacement() const 
    {
        return neg_inf_replacement_;
    }

//  Mutators

    void indenting(bool value)
    {
        indenting_ = value;
    }

    void indent(size_t value)
    {
        indent_ = value;
    }

    void precision(std::streamsize prec)
    {
        precision_ = prec; 
    }

    void escape_all_non_ascii(bool value)
    {
        escape_all_non_ascii_ = value;
    }

    void escape_solidus(bool value)
    {
        escape_solidus_ = value;
    }

    void replace_nan(bool replace)
    {
        replace_nan_ = replace;
    }

    void replace_inf(bool replace)
    {
        replace_pos_inf_ = replace;
        replace_neg_inf_ = replace;
    }

    void replace_pos_inf(bool replace)
    {
        replace_pos_inf_ = replace;
    }

    void replace_neg_inf(bool replace)
    {
        replace_neg_inf_ = replace;
    }

    void nan_replacement(const std::basic_string<Char>& replacement)
    {
        nan_replacement_ = replacement;
    }

    void pos_inf_replacement(const std::basic_string<Char>& replacement)
    {
        pos_inf_replacement_ = replacement;
    }

    void neg_inf_replacement(const std::basic_string<Char>& replacement)
    {
        neg_inf_replacement_ = replacement;
    }
private:
    bool indenting_;
    size_t indent_;
    std::streamsize precision_;

    bool replace_nan_;
    bool replace_pos_inf_;
    bool replace_neg_inf_;
    std::basic_string<Char> nan_replacement_;
    std::basic_string<Char> pos_inf_replacement_;
    std::basic_string<Char> neg_inf_replacement_;
    bool escape_all_non_ascii_;
    bool escape_solidus_;
};

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

        os.precision(16);
    }
    basic_json_stream_writer(std::basic_ostream<Char>& os, basic_output_format<Char> format)
        : os_(os), format_(format), indent_(0)
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();

        os.precision(format_.precision());
    }

    ~basic_json_stream_writer()
    {
        os_.precision(original_precision_);
        os_.flags(original_format_flags_);
    }

    void begin_member(const Char* name, size_t length)
    {
        if (stack_.back().count_ > 0)
        {
            os_ << ',';
        }
        write_indent();
        os_ << '\"'; 
        escape_string<Char>(name,length,format_,os_); 
        os_ << '\"' 
            << ':';
    }

    void end_member()
    {
        ++stack_.back().count_;
    }

    void begin_element()
    {
        if (stack_.back().count_ > 0)
        {
            os_ << ',';
        }
        write_indent();
    }

    void end_element()
    {
        ++stack_.back().count_;
    }

    void value(const Char* value, size_t length)
    {
        os_ << '\"';
        escape_string<Char>(value,length,format_,os_);
        os_ << '\"';
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
        os_ << value ? json_char_traits<Char>::true_literal() :  json_char_traits<Char>::false_literal();
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
        os_ << "{";
        indent();
    }

    void end_object()
    {
        deindent();
        write_indent();
        stack_.pop_back();
        os_ << '}';
    }

    void begin_array()
    {
        //write_indent();
        stack_.push_back(stack_item());
        os_ << "[";
        indent();
    }

    void end_array()
    {
        deindent();
        write_indent();
        stack_.pop_back();
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
        if (format_.indenting() && stack_.size() > 0)
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
