// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_DECODER_HPP
#define JSONCONS_JSON_DECODER_HPP

#include <cstddef>
#include <cstdint>
#include <memory> // std::allocator
#include <system_error>
#include <utility> // std::move
#include <vector>

#include <jsoncons/json_object.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/json_literals.hpp>

namespace jsoncons {

template <typename Json,typename TempAlloc =std::allocator<char>>
class json_decoder final : public basic_json_visitor<typename Json::char_type>
{
public:
    using char_type = typename Json::char_type;
    using typename basic_json_visitor<char_type>::string_view_type;

    using key_value_type = typename Json::key_value_type;
    using key_type = typename Json::key_type;
    using array = typename Json::array;
    using object = typename Json::object;
    using allocator_type = typename Json::allocator_type;
    using json_string_allocator = typename key_type::allocator_type;
    using json_array_allocator = typename array::allocator_type;
    using json_object_allocator = typename object::allocator_type;
    using json_byte_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint8_t>;
private:
    enum class json_structure_kind {root_kind, array_kind, object_kind};

    struct json_structure
    {
        json_structure_kind structure_kind;
        std::size_t structure_index{0};
        int is_key{0};

        json_structure(json_structure_kind type, std::size_t offset) noexcept
            : structure_kind(type), structure_index(offset)
        {
        }
        ~json_structure() = default;
    };

    using temp_allocator_type = TempAlloc;
    using object_member_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<index_key_value<Json>>;
    using structure_info_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<json_structure>;
 
    allocator_type allocator_;

    Json result_;

    std::size_t index_{0};
    key_type name_;
    std::vector<index_key_value<Json>,object_member_allocator_type> item_stack_;
    std::vector<json_structure,structure_info_allocator_type> structure_stack_;
    bool is_valid_{false};

public:
    json_decoder(const allocator_type& alloc = allocator_type(), 
        const temp_allocator_type& temp_alloc = temp_allocator_type())
        : allocator_(alloc),
          result_(),
          name_(alloc),
          item_stack_(alloc),
          structure_stack_(temp_alloc)
    {
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    json_decoder(temp_allocator_arg_t, 
        const temp_allocator_type& temp_alloc = temp_allocator_type())
        : allocator_(),
          result_(),
          name_(),
          item_stack_(),
          structure_stack_(temp_alloc)
    {
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    void reset()
    {
        is_valid_ = false;
        index_ = 0;
        item_stack_.clear();
        structure_stack_.clear();
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    bool is_valid() const
    {
        return is_valid_;
    }

    Json get_result()
    {
        JSONCONS_ASSERT(is_valid_);
        is_valid_ = false;
        return std::move(result_);
    }

private:

    void visit_flush() override
    {
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag tag, const ser_context&, std::error_code&) override
    {
        if (structure_stack_.back().structure_kind == json_structure_kind::object_kind)
        {
            structure_stack_.back().is_key = !structure_stack_.back().is_key;
            structure_stack_.emplace_back(json_structure_kind::object_kind, item_stack_.size());
            item_stack_.emplace_back(std::move(name_), index_++, json_object_arg, tag);
        }
        else
        {
            structure_stack_.emplace_back(json_structure_kind::object_kind, item_stack_.size());
            item_stack_.emplace_back(key_type(allocator_), 0, json_object_arg, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(structure_stack_.size() > 0);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::object_kind);
        const size_t structure_index = structure_stack_.back().structure_index;
        JSONCONS_ASSERT(item_stack_.size() > structure_index);
        const size_t count = item_stack_.size() - (structure_index + 1);
        auto first = item_stack_.begin() + (structure_index+1);

        if (count > 0)
        {
            if (structure_stack_.back().is_key)
            {
            }
            else
            {
                item_stack_[structure_index].value.template cast<typename Json::object_storage>().value().uninitialized_init(
                    &item_stack_[structure_index+1], count);
            }
        }

        item_stack_.erase(first, item_stack_.end());
        structure_stack_.pop_back();
        if (structure_stack_.back().structure_kind == json_structure_kind::root_kind)
        {
            result_.swap(item_stack_.front().value);
            item_stack_.pop_back();
            is_valid_ = true;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag tag, const ser_context&, std::error_code&) override
    {
        if (structure_stack_.back().structure_kind == json_structure_kind::object_kind)
        {
            structure_stack_.back().is_key = !structure_stack_.back().is_key;
            structure_stack_.emplace_back(json_structure_kind::array_kind, item_stack_.size());
            item_stack_.emplace_back(std::move(name_), index_++, json_array_arg, tag);
        }
        else
        {
            structure_stack_.emplace_back(json_structure_kind::array_kind, item_stack_.size());
            item_stack_.emplace_back(key_type(allocator_), 0, json_array_arg, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(structure_stack_.size() > 1);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::array_kind);
        const size_t structure_index = structure_stack_.back().structure_index;
        JSONCONS_ASSERT(item_stack_.size() > structure_index);

        auto& container = item_stack_[structure_index].value;

        const size_t size = item_stack_.size() - (structure_index + 1);
        //std::cout << "size on item stack: " << size << "\n";

        if (size > 0)
        {
            if (structure_stack_.back().is_key)
            {
            }
            else
            {
                container.reserve(size);
                auto first = item_stack_.begin() + (structure_index+1);
                auto last = first + size;
                for (auto it = first; it != last; ++it)
                {
                    container.push_back(std::move((*it).value));
                }
                item_stack_.erase(first, item_stack_.end());
            }
        }

        structure_stack_.pop_back();
        if (structure_stack_.back().structure_kind == json_structure_kind::root_kind)
        {
            result_.swap(item_stack_.front().value);
            item_stack_.pop_back();
            is_valid_ = true;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type& sv, const ser_context&, std::error_code&) override
    {
        structure_stack_.back().is_key = !structure_stack_.back().is_key;
        JSONCONS_ASSERT(structure_stack_.back().is_key);
        name_ = key_type(sv.data(),sv.length(),allocator_);
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& sv, semantic_tag tag, const ser_context&, std::error_code&) override
    {
        auto& structure = structure_stack_.back();
        switch (structure.structure_kind)
        {
            case json_structure_kind::object_kind:
                if ((structure.is_key = !structure.is_key))
                {
                    name_ = key_type(sv.data(),sv.length(),allocator_);
                }
                else
                {
                    item_stack_.emplace_back(std::move(name_), index_++, sv, tag);
                }
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, sv, tag);
                break;
            default:
                result_ = Json(sv, tag, allocator_);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context&,
                           std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, byte_string_arg, b, tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, byte_string_arg, b, tag);
                break;
            default:
                result_ = Json(byte_string_arg, b, tag, allocator_);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& b, 
        uint64_t ext_tag, 
        const ser_context&,
        std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, byte_string_arg, b, ext_tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, byte_string_arg, b, ext_tag);
                break;
            default:
                result_ = Json(byte_string_arg, b, ext_tag, allocator_);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value, 
        semantic_tag tag, 
        const ser_context&,
        std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, value, tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, value, tag);
                break;
            default:
                result_ = Json(value,tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value, 
        semantic_tag tag, 
        const ser_context&,
        std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, value, tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, value, tag);
                break;
            default:
                result_ = Json(value,tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_half(uint16_t value, 
        semantic_tag tag,   
        const ser_context&,
        std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, half_arg, value, tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, half_arg, value, tag);
                break;
            default:
                result_ = Json(half_arg, value, tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_double(double value, 
        semantic_tag tag,   
        const ser_context&,
        std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                structure_stack_.back().is_key = !structure_stack_.back().is_key;
                item_stack_.emplace_back(std::move(name_), index_++, value, tag);
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, value, tag);
                break;
            default:
                result_ = Json(value, tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, semantic_tag tag, const ser_context&, std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                if ((structure_stack_.back().is_key = !structure_stack_.back().is_key))
                {
                    auto sv = value ? json_literals<char_type>::true_literal : json_literals<char_type>::false_literal;
                    name_ = key_type(sv.data(), sv.length(),allocator_);
                }
                else
                {
                    structure_stack_.back().is_key = !structure_stack_.back().is_key;
                    item_stack_.emplace_back(std::move(name_), index_++, value, tag);
                }
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, value, tag);
                break;
            default:
                result_ = Json(value, tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag tag, const ser_context&, std::error_code&) override
    {
        switch (structure_stack_.back().structure_kind)
        {
            case json_structure_kind::object_kind:
                if ((structure_stack_.back().is_key = !structure_stack_.back().is_key))
                {
                    name_ = key_type(json_literals<char_type>::null_literal.data(),json_literals<char_type>::null_literal.length(),allocator_);
                }
                else
                {
                    item_stack_.emplace_back(std::move(name_), index_++, null_type(), tag);
                }
                break;
            case json_structure_kind::array_kind:
                item_stack_.emplace_back(key_type(allocator_), 0, null_type(), tag);
                break;
            default:
                result_ = Json(null_type(), tag);
                is_valid_ = true;
                break;
        }
        JSONCONS_VISITOR_RETURN;
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_DECODER_HPP
