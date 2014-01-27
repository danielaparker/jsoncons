// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_EXT_BSON_SERIALIZER_HPP
#define JSONCONS_EXT_BSON_SERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_output_handler.hpp"
#include <limits> // std::numeric_limits

namespace jsoncons_ext { namespace bson {

template <class C>
class basic_bson_serializer : public jsoncons::basic_json_output_handler<C>
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
    basic_bson_serializer(std::basic_ostream<C>& os)
        : os_(os), size_(4)
    {
    }

    ~basic_bson_serializer()
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

        stack_.push_back(stack_item(true));
        //os_.put('{');
    }

    virtual void end_object()
    {
        stack_.pop_back();
        //os_.put('}');

        end_value();
    }

    virtual void begin_array()
    {
        begin_structure();

        stack_.push_back(stack_item(false));
        //os_.put('[');
    }

    virtual void end_array()
    {
        stack_.pop_back();
        //os_.put(']');

        end_value();
    }

    virtual void name(const std::basic_string<C>& name)
    {
        begin_element();
        //os_.put('\"'); 
        //escape_string<C>(name,format_,os_); 
        //os_.put('\"'); 
        //os_.put(':');
    }

    virtual void value(const std::basic_string<C>& value)
    {
        begin_value();

        //os_.put('\"');
        //escape_string<C>(value,format_,os_);
        //os_.put('\"');

        end_value();
    }

    virtual void value(double value)
    {
        begin_value();

        /*if (is_nan(value) && format_.replace_nan())
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
        }*/

        end_value();
    }

    virtual void value(long long value)
    {
        begin_value();

        os_  << value;

        end_value();
    }

    virtual void value(unsigned long long value)
    {
        begin_value();

        os_  << value;

        end_value();
    }

    virtual void value(bool value)
    {
        begin_value();

        os_ << (value ? jsoncons::json_char_traits<C>::true_literal() :  jsoncons::json_char_traits<C>::false_literal());

        end_value();
    }

    virtual void null_value()
    {
        begin_value();

        os_ << jsoncons::json_char_traits<C>::null_literal();

        end_value();
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

    std::basic_ostream<C>& os_;
    std::vector<stack_item> stack_;
    size_t size_;
    std::vector<unsigned char> document_;
private:
};

typedef basic_bson_serializer<char> bson_serializer;

}}

#endif
