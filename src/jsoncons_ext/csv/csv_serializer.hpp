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
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons_ext { namespace csv {

template <class Char>
struct csv_char_traits
{
};

template <>
struct csv_char_traits<char>
{
    static bool contains_char(const std::string& s, char c)
    {
        size_t pos = s.find_first_of(c);
        return pos == std::string::npos ? false : true;
    }

    static const std::string all_literal() {return "all";};

    static const std::string minimal_literal() {return "minimal";};

    static const std::string none_literal() {return "none";};

    static const std::string nonnumeric_literal() {return "nonumeric";};
};

template <>
struct csv_char_traits<wchar_t>
{
    static bool contains_char(const std::wstring& s, wchar_t c)
    {
        size_t pos = s.find_first_of(c);
        return pos == std::string::npos ? false : true;
    }

    static const std::wstring all_literal() {return L"all";};

    static const std::wstring minimal_literal() {return L"minimal";};

    static const std::wstring none_literal() {return L"none";};

    static const std::wstring nonnumeric_literal() {return L"nonumeric";};
};

template <class Char>
void escape_string(const std::basic_string<Char>& s, 
                   Char quote_char, Char quote_escape_char,
                   std::basic_ostream<Char>& os)
{
    std::basic_string<Char>::const_iterator begin = s.begin();
    std::basic_string<Char>::const_iterator end = s.end();
    for (std::basic_string<Char>::const_iterator it = begin; it != end; ++it)
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

template<class Char>
class basic_csv_serializer : public jsoncons::basic_json_output_handler<Char>
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
    enum quote_style_enum{quote_all,quote_minimal,quote_none,quote_nonnumeric};
public:
    basic_csv_serializer(std::basic_ostream<Char>& os)
       : os_(os), line_delimiter_("\n"), field_delimiter_(','), quote_char_('\"'), quote_escape_char_('\"'), quote_style_(quote_minimal)
    {
        init();
    }

    basic_csv_serializer(std::basic_ostream<Char>& os,
                         const jsoncons::basic_json<Char>& params)
       : os_(os), field_delimiter_(','), quote_char_('\"'), quote_escape_char_('\"')
    {

        line_delimiter_ = params.get("line_delimiter","\n").as_string();
        field_delimiter_ = params.get("field_delimiter",",").as_char();
        quote_char_ = params.get("quote_char","\"").as_char();
        quote_escape_char_ = params.get("quote_escape_char","\"").as_char();
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
        init();
    }

/*    basic_csv_serializer(std::basic_ostream<Char>& os, const jsoncons::basic_output_format<Char>& format)
       : os_(os), format_(format), indent_(0),
         indenting_(format.indenting()) // Deprecated behavior
    {
        original_precision_ = os.precision();
        original_format_flags_ = os.flags();
        init();
    }
*/
    ~basic_csv_serializer()
    {
        restore();
    }

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object()
    {
        stack_.push_back(stack_item(true));
    }

    virtual void end_object()
    {
        if (stack_.size() == 2)
        {
            os_ << line_delimiter_;
            if (stack_[0].count_ == 0)
            {
                os_ << header_os_.str() << line_delimiter_;
            }
        }
        stack_.pop_back();

        end_value();
    }

    virtual void begin_array()
    {
        stack_.push_back(stack_item(false));
    }

    virtual void end_array()
    {
        if (stack_.size() == 2)
        {
            os_ << line_delimiter_;
        }
        stack_.pop_back();

        end_value();
    }

    virtual void name(const std::basic_string<Char>& name)
    {
        if (stack_.size() == 2 && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                os_.put(field_delimiter_);
            }
            bool quote = false;
            if (quote_style_ == quote_all || quote_style_ == quote_nonnumeric ||
                (quote_style_ == quote_minimal && csv_char_traits<Char>::contains_char(name,field_delimiter_)))
            {
                quote = true;
                os_.put(quote_char_);
            }
            jsoncons_ext::csv::escape_string<Char>(name, quote_char_, quote_escape_char_, os_);
            if (quote)
            {
                os_.put(quote_char_);
            }
        }
    }

    virtual void value(const std::basic_string<Char>& value)
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                header_os_.put(field_delimiter_);
            }
            bool quote = false;
            if (quote_style_ == quote_all || quote_style_ == quote_nonnumeric ||
                (quote_style_ == quote_minimal && csv_char_traits<Char>::contains_char(value,field_delimiter_)))
            {
                quote = true;
                header_os_.put(quote_char_);
            }
            jsoncons_ext::csv::escape_string<Char>(value, quote_char_, quote_escape_char_, header_os_);
            if (quote)
            {
                header_os_.put(quote_char_);
            }
            ++stack_.back().count_;
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            bool quote = false;
            if (quote_style_ == quote_all || quote_style_ == quote_nonnumeric ||
                (quote_style_ == quote_minimal && csv_char_traits<Char>::contains_char(value,field_delimiter_)))
            {
                quote = true;
                os_.put(quote_char_);
            }
            jsoncons_ext::csv::escape_string<Char>(value, quote_char_, quote_escape_char_, os_);
            if (quote)
            {
                os_.put(quote_char_);
            }

            end_value();
        }
    }

    virtual void value(double value)
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                header_os_.put(field_delimiter_);
                header_os_  << value;
            }
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            if (jsoncons::is_nan(value) && format_.replace_nan())
            {
                os_  << format_.nan_replacement();
            }
            else if (jsoncons::is_pos_inf(value) && format_.replace_pos_inf())
            {
                os_  << format_.pos_inf_replacement();
            }
            else if (jsoncons::is_neg_inf(value) && format_.replace_neg_inf())
            {
                os_  << format_.neg_inf_replacement();
            }
            else if (format_.truncate_trailing_zeros_notation())
            {
                char buffer[32];
                int len = jsoncons::c99_snprintf(buffer, 32, "%#.*g", format_.precision(), value);
                while (len >= 2 && buffer[len - 1] == '0' && buffer[len - 2] != '.')
                {
                    --len;
                }
                buffer[len] = 0;
                os_ << buffer;
            }
            else
            {
                os_  << value;
            }
            
            end_value();
        }
    }

    virtual void value(long long value)
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                header_os_.put(field_delimiter_);
                header_os_  << value;
            }
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            os_  << value;

            end_value();
        }
    }

    virtual void value(unsigned long long value)
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                header_os_.put(field_delimiter_);
                header_os_  << value;
            }
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            os_  << value;

            end_value();
        }
    }

    virtual void value(bool value)
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
            if (stack_.back().count_ > 0)
            {
                header_os_.put(field_delimiter_);
                header_os_ << (value ? jsoncons::json_char_traits<Char>::true_literal() :  jsoncons::json_char_traits<Char>::false_literal());
            }
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            os_ << (value ? jsoncons::json_char_traits<Char>::true_literal() :  jsoncons::json_char_traits<Char>::false_literal());

            end_value();
        }
    }

    virtual void null_value()
    {
        if (stack_.size() == 2 && stack_.back().is_object() && stack_[0].count_ == 0)
        {
        }
        else if (stack_.size() == 2)
        {
            begin_value();

            os_ << jsoncons::json_char_traits<Char>::null_literal();

            end_value();
        }
    }
private:

    void begin_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                os_.put(field_delimiter_);
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


    std::basic_ostream<Char>& os_;
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
};

typedef basic_csv_serializer<char> csv_serializer;

}}

#endif
