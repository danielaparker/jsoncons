// Copyright 2013-2016 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_DECODER_HPP
#define JSONCONS_JSON_DECODER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_input_handler.hpp>

namespace jsoncons {

template <class Json,template <class Type> class Allocator=std::allocator>
class json_decoder : public basic_json_input_handler<typename Json::char_type>
{
public:
    typedef typename Json::char_type char_type;
    using typename basic_json_input_handler<char_type>::string_view_type;

    static const int default_stack_size = 1000;

    typedef typename Json::key_value_pair_type key_value_pair_type;
    typedef typename Json::key_storage_type key_storage_type;
    typedef typename Json::string_type string_type;
    typedef typename Json::array array;
    typedef typename Json::object object;
    typedef typename Json::allocator_type json_allocator_type;
    typedef typename string_type::allocator_type json_string_allocator;
    typedef typename array::allocator_type json_array_allocator;
    typedef typename object::allocator_type json_object_allocator;

    json_string_allocator string_allocator_;
    json_object_allocator object_allocator_;
    json_array_allocator array_allocator_;

    Json result_;
    size_t top_;

    struct stack_item
    {
        key_storage_type name_;
        Json value_;
    };
    typedef Allocator<stack_item> stack_item_allocator;
    typedef Allocator<size_t> size_t_allocator;

    std::vector<stack_item,stack_item_allocator> stack_;
    std::vector<size_t,size_t_allocator> stack_offsets_;
    bool is_valid_;

public:
    json_decoder(const json_allocator_type& jallocator = json_allocator_type())
        : string_allocator_(jallocator),
          object_allocator_(jallocator),
          array_allocator_(jallocator),
          top_(0),
          stack_(default_stack_size),
          stack_offsets_(),
          is_valid_(false) 

    {
        stack_offsets_.reserve(100);
    }

    bool is_valid() const
    {
        return is_valid_;
    }

    Json get_result()
    {
        is_valid_ = false;
        return std::move(result_);
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    Json& root()
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
        stack_offsets_.push_back(top_);
        stack_[top_].value_ = object(object_allocator_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void pop_object()
    {
        stack_offsets_.pop_back();
        JSONCONS_ASSERT(top_ > 0);
    }

    void push_array()
    {
        stack_offsets_.push_back(top_);
        stack_[top_].value_ = array(array_allocator_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void pop_array()
    {
        stack_offsets_.pop_back();
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

    void do_begin_object(const parsing_context&) override
    {
        push_object();
    }

    void do_end_object(const parsing_context&) override
    {
        end_structure();
        pop_object();
    }

    void do_begin_array(const parsing_context&) override
    {
        push_array();
    }

    void do_end_array(const parsing_context&) override
    {
        end_structure();
        pop_array();
    }

    void end_structure() 
    {
        JSONCONS_ASSERT(stack_offsets_.size() > 0);
        const size_t structure_index = stack_offsets_.back();
        const size_t count = top_ - (structure_index + 1);
        auto first = stack_.begin() + (structure_index+1);
        auto last = first + count;
        if (stack_[stack_offsets_.back()].value_.is_object())
        {
            stack_[structure_index].value_.object_value().insert(
                std::make_move_iterator(first),
                std::make_move_iterator(last),
                [](stack_item&& val){return key_value_pair_type(std::move(val.name_),std::move(val.value_));});
        }
        else
        {
            auto& j = stack_[structure_index].value_;
            j.reserve(count);
            while (first != last)
            {
                j.push_back(std::move(first->value_));
                ++first;
            }
        }
        top_ -= count;
    }

    void do_name(const string_view_type& name, const parsing_context&) override
    {
        stack_[top_].name_ = key_storage_type(name.begin(),name.end(),string_allocator_);
    }

    void do_string_value(const string_view_type& val, const parsing_context&) override
    {
        stack_[top_].value_ = Json(val.data(),val.length(),string_allocator_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const parsing_context&) override
    {
        stack_[top_].value_ = Json(data,length,string_allocator_);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_integer_value(int64_t value, const parsing_context&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_uinteger_value(uint64_t value, const parsing_context&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_double_value(double value, uint8_t precision, const parsing_context&) override
    {
        stack_[top_].value_ = Json(value,precision);
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_bool_value(bool value, const parsing_context&) override
    {
        stack_[top_].value_ = value;
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }

    void do_null_value(const parsing_context&) override
    {
        stack_[top_].value_ = Json::null();
        if (++top_ >= stack_.size())
        {
            stack_.resize(top_*2);
        }
    }
};

}

#endif
