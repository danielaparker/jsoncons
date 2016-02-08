// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_DESERIALIZER_HPP
#define JSONCONS_JSON_DESERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"

namespace jsoncons {

template <class JsonT>
class basic_json_deserializer : public basic_json_input_handler<typename JsonT::char_type>
{
    static const int default_depth = 100;

    typedef typename JsonT::char_type char_type;
    typedef typename JsonT::member_type member_type;
    typedef typename member_type::string_type string_type;
    typedef typename JsonT::allocator_type allocator_type;
    typedef typename JsonT::array array;
    typedef typename JsonT::object object;
    typedef typename array::value_type value_type;

    struct stack_item
    {
        member_type member;
        JsonT value;
    };

public:
    basic_json_deserializer(const allocator_type allocator = allocator_type())
        : result_(allocator),
          top_(-1),
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

#if !defined(JSONCONS_NO_DEPRECATED)
    JsonT& root()
    {
        return result_;
    }
#endif

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
        stack_[top_].value = object(result_.get_allocator());
    }

    void push_array()
    {
        ++top_;
        if (top_ >= depth_)
        {
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_].value = array(result_.get_allocator());
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

    void do_begin_object(const basic_parsing_context<char_type>&) override
    {
        push_object();
    }

    void do_end_object(const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value.object_value().end_bulk_insert();
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].member.value(std::move(stack_[top_].value));
                stack_[top_-1].value.object_value().bulk_insert(std::move(stack_[top_-1].member));
            }
            else
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
        (void)context;
        push_array();
    }

    void do_end_array(const basic_parsing_context<char_type>&) override
    {
        if (top_ > 0)
        {
            if (stack_[top_-1].value.is_object())
            {
                stack_[top_-1].member.value(std::move(stack_[top_].value));
                stack_[top_-1].value.object_value().bulk_insert(std::move(stack_[top_-1].member));
            }
            else
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
        stack_[top_].member = member_type(string_type(p,length));
    }

    void do_string_value(const char_type* p, size_t length, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_string(p,length);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].member.value(value_type(p,length,result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(JsonT(p,length,result_.get_allocator()));
        }
    }

    void do_integer_value(int64_t value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_integer(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].member.value(value_type(value,result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(value_type(value,result_.get_allocator()));
        }
    }

    void do_uinteger_value(uint64_t value, const basic_parsing_context<char_type>&) override
    {
        if (top_ == -1)
        {
            result_.assign_uinteger(value);
        }
        else if (stack_[top_].value.is_object())
        {
            stack_[top_].member.value(value_type(value,result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(value_type(value,result_.get_allocator()));
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
            stack_[top_].member.value(value_type(value,result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(value_type(value,result_.get_allocator()));
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
            stack_[top_].member.value(value_type(value,result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(value_type(value,result_.get_allocator()));
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
            stack_[top_].member.value(value_type(null_type(),result_.get_allocator()));
            stack_[top_].value.object_value().bulk_insert(std::move(stack_[top_].member));
        } 
        else
        {
            stack_[top_].value.array_value().push_back(value_type(null_type(),result_.get_allocator()));
        }
    }

    int top_;
    std::vector<stack_item> stack_;
    int depth_;
    bool is_valid_;
};

}

#endif
