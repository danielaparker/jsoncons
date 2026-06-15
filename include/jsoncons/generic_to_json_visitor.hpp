// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_GENERIC_TO_JSON_VISITOR_HPP
#define JSONCONS_GENERIC_TO_JSON_VISITOR_HPP

#include <cstddef>
#include <cstdint>
#include <memory> // std::allocator
#include <system_error>
#include <utility> // std::move
#include <vector>

#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/generic_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/json_literals.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/write_number.hpp>

namespace jsoncons {

template <typename CharT,typename Allocator =std::allocator<char>>
class basic_generic_to_json_visitor2 final : public basic_generic_visitor<CharT>
{
public:
    using allocator_type = Allocator;
    using typename basic_generic_visitor<CharT>::char_type;
    using typename basic_generic_visitor<CharT>::string_view_type;
private:
    using char_allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<char_type>;
    using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;

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

    using string_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type>;
    using json_structure_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_structure>;

    basic_default_json_visitor<char_type> default_visitor_;
    basic_json_visitor<char_type>* destination_;
    std::size_t index_{0};
    string_type key_;
    std::vector<string_type,string_allocator_type> key_part_stack_;
    std::vector<json_structure,json_structure_allocator_type> structure_stack_;

    // noncopyable and nonmoveable
    basic_generic_to_json_visitor2& operator=(const basic_generic_to_json_visitor2&) = delete;
    basic_generic_to_json_visitor2(const basic_generic_to_json_visitor2&) = delete;
public:
    basic_generic_to_json_visitor2(const allocator_type& alloc = allocator_type())
        : default_visitor_(), 
          destination_(std::addressof(default_visitor_)),
          key_(alloc),
          key_part_stack_(alloc),
          structure_stack_(alloc)
    {
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    basic_generic_to_json_visitor2(basic_json_visitor<char_type>& visitor, 
        const Allocator& alloc = Allocator())
        : destination_(std::addressof(visitor)),
          key_(alloc),
          key_part_stack_(alloc),
          structure_stack_(alloc)
    {
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    void reset()
    {
        key_.clear();
        index_ = 0;
        key_part_stack_.clear();
        structure_stack_.clear();
        structure_stack_.emplace_back(json_structure_kind::root_kind, 0);
    }

    basic_json_visitor<char_type>& destination()
    {
        return *destination_;
    }

    void destination(basic_json_visitor<char_type>& dest)
    {
        destination_ = std::addressof(dest);
    }

private:

    bool is_composite_key() const
    {
        return !key_part_stack_.empty();
    }

    void visit_flush() final
    {
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag tag, 
        const ser_context& context, 
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure_stack_.back().is_key;
            structure_stack_.emplace_back(json_structure_kind::object_kind, key_part_stack_.size());
            if (structure.is_key)
            {
                key_part_stack_.emplace_back();
            }
        }
        else
        {
            structure_stack_.emplace_back(json_structure_kind::object_kind, key_part_stack_.size());
            destination_->begin_object(tag, context, ec);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context& context, 
        std::error_code& ec) final
    {
        JSONCONS_ASSERT(structure_stack_.size() > 0);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::object_kind);
        //JSONCONS_ASSERT(key_part_stack_.size() > structure_index);
        //const size_t size = key_part_stack_.size() - (structure_index + 1);

        auto& structure = structure_stack_[structure_stack_.size()-2];
        //auto* keys = key_part_stack_.data() + structure_index;

        if (!is_composite_key())
        {
            destination_->end_object(context, ec);
        }
        else
        {
            const size_t structure_index = structure_stack_.back().structure_index;
            JSONCONS_ASSERT(key_part_stack_.size() > structure_index);
            auto& key = key_part_stack_[structure_index];
            auto first = key_part_stack_.begin() + (structure_index+1);
            auto last = key_part_stack_.end();

            bool start = true;
            for (auto it = first; it != last; ++it)
            {
                if (!start)
                {
                    key.push_back(',');
                }
                else
                {
                    start = false;
                }
                key.append(*it);
            }
            key_part_stack_.erase(first, key_part_stack_.end());
            key_part_stack_.pop_back();
        }

        structure_stack_.pop_back();
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag /*tag*/, 
        const ser_context&, 
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure_stack_.back().is_key;
            structure_stack_.emplace_back(json_structure_kind::array_kind, key_part_stack_.size());
            if (structure.is_key)
            {
                key_part_stack_.emplace_back();
            }
        }
        else
        {
            structure_stack_.emplace_back(json_structure_kind::array_kind, key_part_stack_.size());
            //key_part_stack_.emplace_back(string_type(alloc_), 0, json_array_arg, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, 
        std::error_code&) final
    {
        JSONCONS_ASSERT(structure_stack_.size() > 1);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::array_kind);
        const size_t structure_index = structure_stack_.back().structure_index;
        JSONCONS_ASSERT(key_part_stack_.size() > structure_index);

        auto& structure = structure_stack_[structure_stack_.size()-2];
        auto& arr = key_part_stack_[structure_index];
        const size_t size = key_part_stack_.size() - (structure_index + 1);

        if (size > 0)
        {
            arr.reserve(size);
            auto first = key_part_stack_.begin() + (structure_index+1);
            auto last = first + size;
            for (auto it = first; it != last; ++it)
            {
                //arr.push_back(std::move((*it)));
            }
            key_part_stack_.erase(first, key_part_stack_.end());
        }
        if (structure.is_key)
        {
            key_.clear();
            key_part_stack_.pop_back();
        }

        structure_stack_.pop_back();
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& sv, 
        semantic_tag tag, 
        const ser_context& context, 
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                destination_->key(sv, context, ec);
            }
            else
            {
                destination_->string_value(sv, tag, context, ec);
            }
        }
        else
        {
            key_part_stack_.emplace_back(sv.data(), sv.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
        semantic_tag tag, 
        const ser_context& context,
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                key_.clear();
                switch (tag)
                {
                    case semantic_tag::base64:
                        bytes_to_base64(value.begin(), value.end(), key_);
                        break;
                    case semantic_tag::base16:
                        bytes_to_base16(value.begin(), value.end(), key_);
                        break;
                    default:
                        bytes_to_base64url(value.begin(), value.end(), key_);
                        break;
                }
                destination_->key(key_, context, ec);
            }
            else
            {
                destination_->byte_string_value(value, tag, context, ec);
            }
        }
        else
        {
            key_.clear();
            switch (tag)
            {
                case semantic_tag::base64:
                    bytes_to_base64(value.begin(), value.end(), key_);
                    break;
                case semantic_tag::base16:
                    bytes_to_base16(value.begin(), value.end(), key_);
                    break;
                default:
                    bytes_to_base64url(value.begin(), value.end(), key_);
                    break;
            }
            key_part_stack_.emplace_back(key_.data(), key_.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
        uint64_t ext_tag, 
        const ser_context& context,
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                key_.clear();
                bytes_to_base64url(value.begin(), value.end(), key_);
                destination_->key(key_, context, ec);
            }
            else
            {
                destination_->byte_string_value(value, ext_tag, context, ec);
            }
        }
        else
        {
            key_.clear();
            bytes_to_base64url(value.begin(), value.end(), key_);
            key_part_stack_.emplace_back(key_.data(), key_.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value, 
        semantic_tag tag, 
        const ser_context& context,
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                key_.clear();
                jsoncons::from_integer(value, key_);
                destination_->key(key_, context, ec);
            }
            else
            {
                destination_->int64_value(value, tag, context, ec);
            }
        }
        else
        {
            key_.clear();
            jsoncons::from_integer(value, key_);
            key_part_stack_.emplace_back(key_.data(), key_.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value, 
        semantic_tag tag, 
        const ser_context& context,
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                key_.clear();
                jsoncons::from_integer(value, key_);
                destination_->key(key_, context, ec);
            }
            else
            {
                destination_->uint64_value(value, tag, context, ec);
            }
        }
        else
        {
            key_.clear();
            jsoncons::from_integer(value, key_);
            key_part_stack_.emplace_back(key_.data(), key_.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_half(uint16_t value, 
        semantic_tag tag,   
        const ser_context& context,
        std::error_code& ec) final
    {
        visit_double(binary::decode_half(value),
            tag,
            context,
            ec);
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_double(double value, 
        semantic_tag tag,   
        const ser_context& context,
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                key_.clear();
                string_sink<string_type> sink(key_);
                jsoncons::write_double f{float_chars_format::general,0};
                f(value, sink);
                destination_->key(key_, context, ec);
            }
            else
            {
                destination_->double_value(value, tag, context, ec);
            }
        }
        else
        {
            key_.clear();
            string_sink<string_type> sink(key_);
            jsoncons::write_double f{float_chars_format::general,0};
            f(value, sink);
            key_part_stack_.emplace_back(key_.data(), key_.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, 
        semantic_tag tag, 
        const ser_context& context, 
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                auto sv = value ? json_literals<char_type>::true_literal : json_literals<char_type>::false_literal;
                destination_->key(sv, context, ec);
            }
            else
            {
                destination_->bool_value(value, tag, context, ec);
            }
        }
        else
        {
            auto sv = value ? json_literals<char_type>::true_literal : json_literals<char_type>::false_literal;
            key_part_stack_.emplace_back(sv.data(), sv.size());
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag tag, 
        const ser_context& context, 
        std::error_code& ec) final
    {
        auto& structure = structure_stack_.back();

        if (!is_composite_key())
        {
            if ((structure.is_key = !structure.is_key))
            {
                auto sv = json_literals<char_type>::null_literal;
                destination_->key(sv, context, ec);
            }
            else
            {
                destination_->null_value(tag, context, ec);
            }
        }
        else
        {
            auto sv = json_literals<char_type>::null_literal;
            key_part_stack_.emplace_back(sv.data(), sv.size());
        }
        JSONCONS_VISITOR_RETURN;
    }
};

using generic_to_json_visitor2 = basic_generic_to_json_visitor2<char>;

} // namespace jsoncons

#endif // JSONCONS_GENERIC_TO_JSON_VISITOR_HPP
