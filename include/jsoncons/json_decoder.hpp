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
#include <jsoncons/json_content_handler.hpp>

namespace jsoncons {

template <class Json,class Allocator=std::allocator<typename Json::char_type>>
class json_decoder final : public basic_json_content_handler<typename Json::char_type>
{
public:
    typedef typename Json::char_type char_type;
    using typename basic_json_content_handler<char_type>::string_view_type;

    typedef typename Json::key_value_pair_type key_value_pair_type;
    typedef typename Json::key_storage_type key_storage_type;
    typedef typename Json::string_type string_type;
    typedef typename Json::array array;
    typedef typename Json::object object;
    typedef typename Json::allocator_type json_allocator_type;
    typedef typename string_type::allocator_type json_string_allocator;
    typedef typename array::allocator_type json_array_allocator;
    typedef typename object::allocator_type json_object_allocator;
    typedef typename std::allocator_traits<json_allocator_type>:: template rebind_alloc<uint8_t> json_byte_allocator_type;
private:
    json_string_allocator string_allocator_;
    json_object_allocator object_allocator_;
    json_array_allocator array_allocator_;
    json_byte_allocator_type byte_allocator_;

    Json result_;

    struct stack_item
    {
        stack_item(key_storage_type&& name)
            : name_(std::forward<key_storage_type>(name))
        {
        }
        stack_item(Json&& value)
            : value_(std::forward<Json>(value))
        {
        }

        stack_item() = default;
        stack_item(const stack_item&) = default;
        stack_item(stack_item&&) = default;
        stack_item& operator=(const stack_item&) = default;
        stack_item& operator=(stack_item&&) = default;

        key_storage_type name_;
        Json value_;
    };

    enum class structure_type {root_t, array_t, object_t};

    struct structure_offset
    {
        size_t offset_;
        structure_type type_;
    };

    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<stack_item> stack_item_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<structure_offset> size_t_allocator_type;


    std::vector<stack_item,stack_item_allocator_type> stack_;
    std::vector<structure_offset,size_t_allocator_type> stack_offsets_;
    bool is_valid_;

public:
    json_decoder(const json_allocator_type& jallocator = json_allocator_type())
        : string_allocator_(jallocator),
          object_allocator_(jallocator),
          array_allocator_(jallocator),
          is_valid_(false) 

    {
        stack_.reserve(1000);
        stack_offsets_.reserve(100);
        stack_offsets_.push_back({0,structure_type::root_t});
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

    void do_flush() override
    {
    }

    bool do_begin_object(const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(object(object_allocator_));
                break;
            case structure_type::array_t:
                stack_.push_back(Json(object(object_allocator_)));
                break;
            default:
                stack_.clear();
                is_valid_ = false;
                stack_.push_back(Json(object(object_allocator_)));
                result_ = Json(object(object_allocator_));
                break;
        }
        stack_offsets_.push_back({stack_.size()-1,structure_type::object_t});
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        end_structure();
        return true;
    }

    bool do_begin_array(const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(array(array_allocator_));
                break;
            case structure_type::array_t:
                stack_.push_back(Json(array(array_allocator_)));
                break;
            default:
                stack_.clear();
                is_valid_ = false;
                stack_.push_back(Json(array(array_allocator_)));
                result_ = Json(array(array_allocator_));
                break;
        }
        stack_offsets_.push_back({stack_.size()-1,structure_type::array_t});
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        end_structure();
        return true;
    }

    void end_structure() 
    {
        JSONCONS_ASSERT(stack_offsets_.size() > 0);
        const size_t structure_index = stack_offsets_.back().offset_;
        JSONCONS_ASSERT(stack_.size() > structure_index);
        const size_t count = stack_.size() - (structure_index + 1);

        auto first = stack_.begin() + (structure_index+1);
        auto last = first + count;

        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                {
                    stack_[structure_index].value_.object_value().insert(
                        std::make_move_iterator(first),
                        std::make_move_iterator(last),
                        [](stack_item&& val){return key_value_pair_type(std::move(val.name_),std::move(val.value_));});
                }
                break;
            default:
                {
                    auto& j = stack_[structure_index].value_;
                    j.reserve(count);
                    while (first != last)
                    {
                        j.push_back(std::move(first->value_));
                        ++first;
                    }
                }
                break;
        }
        stack_.erase(stack_.begin()+stack_offsets_.back().offset_+1, stack_.end());
        stack_offsets_.pop_back();
        if (stack_offsets_.back().type_ == structure_type::root_t)
        {
            result_.swap(stack_.front().value_);
            stack_.pop_back();
            is_valid_ = true;
        }
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        stack_.push_back(key_storage_type(name.begin(),name.end(),string_allocator_));
        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type tag, const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(sv, tag, string_allocator_);
                break;
            case structure_type::array_t:
                stack_.push_back(Json(sv, tag, string_allocator_));
                break;
            default:
                result_ = Json(sv, tag, string_allocator_);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_byte_string_value(const uint8_t* data, size_t length, semantic_tag_type, const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(byte_string_view(data,length),byte_allocator_);
                break;
            case structure_type::array_t:
                stack_.push_back(Json(byte_string_view(data,length),byte_allocator_));
                break;
            default:
                result_ = Json(byte_string_view(data,length),byte_allocator_);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag_type tag, 
                        const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(value,tag);
                break;
            case structure_type::array_t:
                stack_.push_back(Json(value,tag));
                break;
            default:
                result_ = Json(value,tag);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag_type tag, 
                         const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(value,tag);
                break;
            case structure_type::array_t:
                stack_.push_back(Json(value,tag));
                break;
            default:
                result_ = Json(value,tag);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_double_value(double value, 
                         const floating_point_options& fmt, 
                         semantic_tag_type tag,   
                         const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json(value, fmt, tag);
                break;
            case structure_type::array_t:
                stack_.push_back(Json(value, fmt, tag));
                break;
            default:
                result_ = Json(value, fmt, tag);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_bool(bool value, const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = value;
                break;
            case structure_type::array_t:
                stack_.push_back(Json(value));
                break;
            default:
                result_ = Json(value);
                is_valid_ = true;
                break;
        }
        return true;
    }

    bool do_null_value(const serializing_context&) override
    {
        switch (stack_offsets_.back().type_)
        {
            case structure_type::object_t:
                stack_.back().value_ = Json::null();
                break;
            case structure_type::array_t:
                stack_.push_back(Json(Json::null()));
                break;
            default:
                result_ = Json(Json::null());
                is_valid_ = true;
                break;
        }
        return true;
    }
};

}

#endif
