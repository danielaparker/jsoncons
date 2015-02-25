// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_EXT_CSV_CSV_SERIALIZER_HPP
#define JSONCONS_EXT_CSV_CSV_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <ostream>
#include <cstdlib>
#include <map>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons_ext { namespace csv {

template <typename Char>
struct csv_char_traits
{
};

template <>
struct csv_char_traits<char>
{
    static const std::string all_literal() {return "all";};

    static const std::string minimal_literal() {return "minimal";};

    static const std::string none_literal() {return "none";};

    static const std::string nonnumeric_literal() {return "nonumeric";};
};

template <>
struct csv_char_traits<wchar_t>
{
    static const std::wstring all_literal() {return L"all";};

    static const std::wstring minimal_literal() {return L"minimal";};

    static const std::wstring none_literal() {return L"none";};

    static const std::wstring nonnumeric_literal() {return L"nonumeric";};
};
 
template <typename Char>
void escape_string(const Char* s,
                   size_t length,
                   Char quote_char, Char quote_escape_char,
                   std::basic_ostream<Char>& os)
{
    const Char* begin = s;
    const Char* end = s + length;
    for (const Char* it = begin; it != end; ++it)
    {
        Char c = *it;
        if (c == quote_char)
        {
            os << quote_escape_char << quote_char;
        }
        else
        {
            os << c;
        }
    }
}

template<typename Char,class Alloc>
class basic_csv_serializer : public jsoncons::basic_json_output_handler<Char>
{
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
    enum quote_style_enum{quote_all,quote_minimal,quote_none,quote_nonnumeric};
public:
    basic_csv_serializer(std::basic_ostream<Char>& os)
       :
       os_(std::addressof(os)),
       format_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       quote_char_('\"'),
       quote_escape_char_('\"'),
       field_delimiter_(','),
       quote_style_(quote_minimal),
       header_os_(),
       line_delimiter_("\n"),
       header_()
    {
    }

    basic_csv_serializer(std::basic_ostream<Char>& os,
                         const jsoncons::basic_json<Char,Alloc>& params)
       :
       os_(std::addressof(os)),
       format_(),
       stack_(),
       original_precision_(),
       original_format_flags_(),
       quote_char_(params.get("quote_char","\"").as_char()),
       quote_escape_char_(params.get("quote_escape_char","\"").as_char()),
       field_delimiter_(params.get("field_delimiter",",").as_char()),
       quote_style_(quote_minimal),
       header_os_(),
       line_delimiter_(params.get("line_delimiter","\n").as_string()),
       header_()
    {

        std::basic_string<Char> quote_style = params.get("quote_style","minimal").as_string();
        if (quote_style == csv_char_traits<Char>::all_literal())
        {
            quote_style_ = quote_all;
        }
        else if (quote_style == csv_char_traits<Char>::minimal_literal())
        {
            quote_style_ = quote_minimal;
        }
        else if (quote_style == csv_char_traits<Char>::none_literal())
        {
            quote_style_ = quote_none;
        }
        else if (quote_style == csv_char_traits<Char>::nonnumeric_literal())
        {
            quote_style_ = quote_nonnumeric;
        }
        else
        {
            JSONCONS_THROW_EXCEPTION("Unrecognized quote style.");
        }
    }

    ~basic_csv_serializer()
    {
    }

private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_begin_object() override
    {
        stack_.push_back(stack_item(true));
    }

    void do_end_object() override
    {
        if (stack_.size() == 2)
        {
            *os_ << line_delimiter_;
            if (stack_[0].count_ == 0)
            {
                *os_ << header_os_.str() << line_delimiter_;
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
            *os_ << line_delimiter_;
        }
        stack_.pop_back();

        end_value();
    }

    void do_name(const Char* name, size_t length) override
    {
        if (stack_.size() == 2)
        {
            if (stack_[0].count_ == 0)
            {
                if (stack_.back().count_ > 0)
                {
                    os_->put(field_delimiter_);
                }
                bool quote = false;
                if (quote_style_ == quote_all || quote_style_ == quote_nonnumeric ||
                    (quote_style_ == quote_minimal && std::char_traits<Char>::find(name,length,field_delimiter_) != nullptr))
                {
                    quote = true;
                    os_->put(quote_char_);
                }
                jsoncons_ext::csv::escape_string<Char>(name, length, quote_char_, quote_escape_char_, *os_);
                if (quote)
                {
                    os_->put(quote_char_);
                }
                header_[name] = stack_.back().count_;
            }
            else
            {
                typename std::map<std::basic_string<Char>,size_t>::iterator it = header_.find(std::basic_string<Char>(name,length));
                if (it == header_.end())
                {
                    stack_.back().skip_ = true;
                    //std::cout << " Not found ";
                }
                else
                {
                    stack_.back().skip_ = false;
                    while (stack_.back().count_ < it->second)
                    {
                        os_->put(field_delimiter_);
                        ++stack_.back().count_;
                    }
                //    std::cout << " (" << it->value() << " " << stack_.back().count_ << ") ";
                }
            }
        }
    }

    void do_null_value() override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                do_null_value(header_os_);
            }
            else
            {
                do_null_value(*os_);
            }
        }
    }

    void do_string_value(const Char* val, size_t length) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                value(val,length,header_os_);
            }
            else
            {
                value(val,length,*os_);
            }
        }
    }

    void do_double_value(double val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,*os_);
            }
        }
    }

    void do_longlong_value(long long val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,*os_);
            }
        }
    }

    void do_ulonglong_value(unsigned long long val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,*os_);
            }
        }
    }

    void do_bool_value(bool val) override
    {
        if (stack_.size() == 2 && !stack_.back().skip_)
        {
            if (stack_.back().is_object() && stack_[0].count_ == 0)
            {
                value(val,header_os_);
            }
            else
            {
                value(val,*os_);
            }
        }
    }

    void value(const Char* val, size_t length, std::basic_ostream<Char>& os)
    {
        begin_value(os);

        bool quote = false;
        if (quote_style_ == quote_all || quote_style_ == quote_nonnumeric ||
            (quote_style_ == quote_minimal && std::char_traits<Char>::find(val,length,field_delimiter_) != nullptr))
        {
            quote = true;
            os.put(quote_char_);
        }
        jsoncons_ext::csv::escape_string<Char>(val, length, quote_char_, quote_escape_char_, os);
        if (quote)
        {
            os.put(quote_char_);
        }

        end_value();
    }

    void value(double val, std::basic_ostream<Char>& os)
    {
        begin_value(os);

        if (jsoncons::is_nan(val) && format_.replace_nan())
        {
            os  << format_.nan_replacement();
        }
        else if (jsoncons::is_pos_inf(val) && format_.replace_pos_inf())
        {
            os  << format_.pos_inf_replacement();
        }
        else if (jsoncons::is_neg_inf(val) && format_.replace_neg_inf())
        {
            os  << format_.neg_inf_replacement();
        }
        else if (format_.floatfield() != 0)
        {
            std::basic_ostringstream<Char> ss;
            ss.imbue(std::locale::classic());
            ss.setf(format_.floatfield(), std::ios::floatfield);
            ss << std::showpoint << std::setprecision(format_.precision()) << val;
            os << ss.str();
        }
        else
        {
            std::basic_string<Char> buf = jsoncons::float_to_string<Char>(val,format_.precision());
            os << buf;
        }

        end_value();

    }

    void value(long long val, std::basic_ostream<Char>& os)
    {
        begin_value(os);

        os  << val;

        end_value();
    }

    void value(unsigned long long val, std::basic_ostream<Char>& os)
    {
        begin_value(os);

        os  << val;

        end_value();
    }

    void value(bool val, std::basic_ostream<Char>& os) 
    {
        begin_value(os);

        os << (val ? jsoncons::json_char_traits<Char,sizeof(Char)>::true_literal() :  jsoncons::json_char_traits<Char,sizeof(Char)>::false_literal());

        end_value();
    }

    void do_null_value(std::basic_ostream<Char>& os) 
    {
        begin_value(os);

        os << jsoncons::json_char_traits<Char,sizeof(Char)>::null_literal();

        end_value();

    }

    void begin_value(std::basic_ostream<Char>& os)
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                os.put(field_delimiter_);
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

    std::basic_ostream<Char>* os_;
    jsoncons::basic_output_format<Char> format_;
    std::vector<stack_item> stack_;
    std::streamsize original_precision_;
    std::ios_base::fmtflags original_format_flags_;
    Char quote_char_;
    Char quote_escape_char_;
    Char field_delimiter_;
    quote_style_enum quote_style_;
    std::basic_ostringstream<Char> header_os_;
    std::basic_string<Char> line_delimiter_;
    std::map<std::basic_string<Char>,size_t> header_;
};

typedef basic_csv_serializer<char,std::allocator<void>> csv_serializer;

}}

#endif
