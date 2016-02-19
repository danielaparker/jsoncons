// Copyright 2013-2016 Daniel Parker
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
    static const int default_stack_size = 1000;

    typedef typename JsonT::char_type char_type;
    typedef typename JsonT::member_type member_type;
    typedef typename JsonT::string_type string_type;
    typedef typename string_type::allocator_type string_allocator;
    typedef typename JsonT::allocator_type allocator_type;
    typedef typename JsonT::array array;
    typedef typename array::allocator_type array_allocator;
    typedef typename JsonT::object object;
    typedef typename object::allocator_type object_allocator;
    typedef typename JsonT::value_type value_type;

    string_allocator sa_;
    object_allocator oa_;
    array_allocator aa_;

    JsonT result_;
    size_t top_;

    struct stack_item
    {
        string_type name_;
        value_type value_;
    };
    std::vector<stack_item> stack_;
    std::vector<size_t> stack2_;
    bool is_valid_;

public:
    basic_json_deserializer(const string_allocator& sa = string_allocator(),
                            const allocator_type& allocator = allocator_type())
        : sa_(sa),
          oa_(allocator),
          aa_(allocator),
          top_(0),
          stack_(default_stack_size),
          stack2_(),
          is_valid_(true) // initial json value is an empty object

    {
        stack2_.reserve(100);
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

    void push_initial()
    {
        top_ = 0;
        if (top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void pop_initial()
    {
        JSONCONS_ASSERT(top_ == 1);
        result_.swap(stack_[0].value_);
        --top_;
    }

    void push_object()
    {
        stack2_.push_back(top_);
        stack_[top_].value_ = object(oa_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void pop_object()
    {
        stack2_.pop_back();
        JSONCONS_ASSERT(top_ > 0);
    }

    void push_array()
    {
        stack2_.push_back(top_);
        stack_[top_].value_ = array(aa_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void pop_array()
    {
        stack2_.pop_back();
        JSONCONS_ASSERT(top_ > 0);
    }

    void do_begin_json() override
    {
        is_valid_ = false;
        push_initial();
    }

    void do_end_json() override
    {
        is_valid_ = true;
        pop_initial();
    }

    void do_begin_object(const basic_parsing_context<char_type>&) override
    {
        push_object();
    }

    void do_end_object(const basic_parsing_context<char_type>&) override
    {
        end_structure();
        pop_object();
    }

    void do_begin_array(const basic_parsing_context<char_type>&) override
    {
        push_array();
    }

    void do_end_array(const basic_parsing_context<char_type>&) override
    {
        end_structure();
        pop_array();
    }

    static member_type move_pair(stack_item&& val)
    {
        return member_type(std::move(val.name_),std::move(val.value_));
    }

    void end_structure() 
    {
        JSONCONS_ASSERT(stack2_.size() > 0);
        if (stack_[stack2_.back()].value_.is_object())
        {
            size_t count = top_ - (stack2_.back() + 1);
            auto s = stack_.begin() + (stack2_.back()+1);
            auto send = s + count;
            stack_[stack2_.back()].value_.object_value().insert(
                std::make_move_iterator(s),
                std::make_move_iterator(send),
                move_pair);
            top_ -= count;
        }
        else
        {
            size_t count = top_ - (stack2_.back() + 1);
            stack_[stack2_.back()].value_.resize(count);

            auto s = stack_.begin() + (stack2_.back()+1);
            auto dend = stack_[stack2_.back()].value_.elements().end();
            for (auto it = stack_[stack2_.back()].value_.elements().begin();
                 it != dend; ++it, ++s)
            {
                *it = std::move(s->value_);
            }
            top_ -= count;
        }
    }

    void do_name(const char_type* p, size_t length, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].name_ = string_type(p,length,sa_);
    }

    void do_string_value(const char_type* p, size_t length, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = JsonT(p,length,sa_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_integer_value(int64_t value, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_uinteger_value(uint64_t value, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_double_value(double value, uint8_t precision, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = value_type(value,precision);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_bool_value(bool value, const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_null_value(const basic_parsing_context<char_type>&) override
    {
        stack_[top_].value_ = null_type();
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }
};

}

#endif
