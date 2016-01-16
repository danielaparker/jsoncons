// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_DESERIALIZER_HPP
#define JSONCONS_JSON_DESERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_structures.hpp"

namespace jsoncons {

template <class JsonT>
class basic_json_deserializer : public basic_json_input_handler<typename JsonT::char_type>
{
    static const int default_depth = 100;

    typedef typename JsonT::char_type char_type;

    struct stack_item
    {
        std::basic_string<char_type> name;
        JsonT value;
    };

public:
    basic_json_deserializer()
        : top_(-1),
          stack_(default_depth),
          depth_(default_depth),
          is_valid_(true) // initial json value is an empty object

    {
    }

    bool is_valid() const
    {
        return is_valid_;
    }

    JsonT get_result()
    {
        is_valid_ = false;
        return std::move(result_);
    }

//  Deprecated
    JsonT& root()
    {
        return result_;
    }

private:
    JsonT result_;

    void push_object()
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].value = JsonT();
    }

    void push_array()
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].value = typename JsonT::make_array();
    }

    void pop_object()
    {
        JSONCONS_ASSERT(top_ >= 0);
        --top_;
    }

    void pop_array()
    {
        JSONCONS_ASSERT(top_ >= 0);
        --top_;
    }

    void do_begin_json() override
    {
        is_valid_ = false;
    }

    void do_end_json() override
    {
        is_valid_ = true;
    }

    void do_begin_object(const basic_parsing_context<char_type>& context) override
    {
        push_object();
    }

    void do_end_object(const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value.object_value().sort_members();
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].value.object_value().push_back(std::move(stack_[top_-1].name),std::move(stack_[top_].value));
            }
            else if (stack_[top_-1].value.is_array()) 
            {
                stack_[top_-1].value.array_value().push_back(std::move(stack_[top_].value));
            }
        }
        else
        {
            result_.swap(stack_[0].value);
        }
        pop_object();
    }

    void do_begin_array(const basic_parsing_context<char_type>& context) override
    {
        push_array();
    }

    void do_end_array(const basic_parsing_context<char_type>&) override
    {
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].value.object_value().push_back(std::move(stack_[top_-1].name),std::move(stack_[top_].value));
            }
            else if (stack_[top_-1].value.is_array()) 
            {
                stack_[top_-1].value.array_value().push_back(std::move(stack_[top_].value));
            }
        }
        else
        {
            result_.swap(stack_[0].value);
        }
        pop_array();
    }

    void do_name(const char_type* p, size_t length, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].name = std::basic_string<char_type>(p,length);
    }

    void do_string_value(const char_type* p, size_t length, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_string(p,length);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),JsonT(p,length));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(JsonT(p,length));
        }
    }

    void do_integer_value(int64_t value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_longlong(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),JsonT(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(value);
        }
    }

    void do_uinteger_value(uint64_t value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_ulonglong(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),JsonT(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(value);
        }
    }

    void do_double_value(double value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_double(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),JsonT(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(value);
        }
    }

    void do_bool_value(bool value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_bool(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),JsonT(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(value);
        }
    }

    void do_null_value(const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_null();
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.object_value().push_back(std::move(stack_[top_].name),std::move(JsonT(null_type())));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.array_value().push_back(JsonT::null);
        }
    }

    int top_;
    std::vector<stack_item> stack_;
    int depth_;
    bool is_valid_;
};

typedef basic_json_deserializer<json> json_deserializer;
typedef basic_json_deserializer<wjson> wjson_deserializer;

}

#endif
