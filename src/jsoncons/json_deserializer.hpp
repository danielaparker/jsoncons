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

template <typename Char,class Alloc>
class basic_json_deserializer : public basic_json_input_handler<Char>
{
    static const int default_depth = 100;

    struct stack_item
    {
        std::basic_string<Char> name;
        basic_json<Char,Alloc> value;
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

    basic_json<Char,Alloc> get_result()
    {
        is_valid_ = false;
        return std::move(result_);
    }

//  Deprecated
    basic_json<Char,Alloc>& root()
    {
        return result_;
    }

private:
    basic_json<Char,Alloc> result_;

    void push_object()
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].value = basic_json<Char,Alloc>();
    }

    void push_array()
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].value = basic_json<Char,Alloc>::make_array();
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

    void do_begin_object(const basic_parsing_context<Char>& context) override
    {
        push_object();
    }

    void do_end_object(const basic_parsing_context<Char>&) override
    {
        stack_[top_].value.members().sort_members();
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].value.members().push_back(std::move(stack_[top_-1].name),std::move(stack_[top_].value));
            }
            else if (stack_[top_-1].value.is_array()) 
            {
                stack_[top_-1].value.elements().push_back(std::move(stack_[top_].value));
            }
        }
        else
        {
            result_.swap(stack_[0].value);
        }
        pop_object();
    }

    void do_begin_array(const basic_parsing_context<Char>& context) override
    {
        push_array();
    }

    void do_end_array(const basic_parsing_context<Char>&) override
    {
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].value.members().push_back(std::move(stack_[top_-1].name),std::move(stack_[top_].value));
            }
            else if (stack_[top_-1].value.is_array()) 
            {
                stack_[top_-1].value.elements().push_back(std::move(stack_[top_].value));
            }
        }
        else
        {
            result_.swap(stack_[0].value);
        }
        pop_array();
    }

    void do_name(const Char* p, size_t length, const basic_parsing_context<Char>&) override
    {
        stack_[top_].name = std::basic_string<Char>(p,length);
    }

    void do_string_value(const Char* p, size_t length, const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_string(p,length);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),basic_json<Char,Alloc>(p,length));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(basic_json<Char,Alloc>(p,length));
        }
    }

    void do_longlong_value(long long value, const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_longlong(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),basic_json<Char,Alloc>(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(value);
        }
    }

    void do_ulonglong_value(unsigned long long value, const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_ulonglong(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),basic_json<Char,Alloc>(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(value);
        }
    }

    void do_double_value(double value, const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_double(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),basic_json<Char,Alloc>(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(value);
        }
    }

    void do_bool_value(bool value, const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_bool(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),basic_json<Char,Alloc>(value));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(value);
        }
    }

    void do_null_value(const basic_parsing_context<Char>&) override
    {
        if (top_ == -1)
        {
            result_.assign_null();
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].value.members().push_back(std::move(stack_[top_].name),std::move(basic_json<Char,Alloc>(null_type())));
        } 
        else if (stack_[top_].value.is_array()) 
        {
            stack_[top_].value.elements().push_back(basic_json<Char,Alloc>::null);
        }
    }

    int top_;
    std::vector<stack_item> stack_;
    int depth_;
    bool is_valid_;
};

typedef basic_json_deserializer<char,std::allocator<void>> json_deserializer;
typedef basic_json_deserializer<wchar_t,std::allocator<void>> wjson_deserializer;

}

#endif
