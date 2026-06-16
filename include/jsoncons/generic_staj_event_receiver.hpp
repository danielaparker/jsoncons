// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_GENERIC_STAJ_EVENT_RECEIVER_HPP
#define JSONCONS_GENERIC_STAJ_EVENT_RECEIVER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/generic_visitor.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_literals.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/staj_event.hpp>
#include <jsoncons/typed_array.hpp>
#include <jsoncons/utility/bigint.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/utility/conversion.hpp>
#include <jsoncons/utility/more_type_traits.hpp>
#include <jsoncons/utility/write_number.hpp>

namespace jsoncons { 

template <typename CharT,typename TempAlloc =std::allocator<char>>
class basic_generic_staj_event_receiver final : public basic_generic_visitor<CharT>
{
public:
    using char_type = CharT;
    using typename basic_generic_visitor<CharT>::string_view_type;
    using staj_event_type = basic_staj_event<CharT>;

private:
    enum class json_structure_kind {root_kind, array_kind, object_kind};

    struct json_structure
    {
        json_structure_kind structure_kind;
        int is_key{0};

        json_structure(json_structure_kind type) noexcept
            : structure_kind(type)
        {
        }
        ~json_structure() = default;
    };

    using temp_allocator_type = TempAlloc;
    using json_structure_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<json_structure>;

    std::vector<json_structure,json_structure_allocator_type> structure_stack_;

public:
    basic_generic_staj_event_receiver(const temp_allocator_type& temp_alloc = temp_allocator_type())
        : event_(staj_events::null_value), structure_stack_(temp_alloc)
    {
        structure_stack_.emplace_back(json_structure_kind::root_kind);
    }

    const staj_event_type& event() const
    {
        return event_;
    }

private:
    staj_event_type event_;

    void visit_flush() final
    {
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure_stack_.back().is_key = !structure_stack_.back().is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(staj_events::begin_object, tag, staj_events::key_flag);
        }
        else 
        {
            event_ = staj_event_type(staj_events::begin_object, tag);
        }
        structure_stack_.emplace_back(json_structure_kind::object_kind);
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) final
    {
        JSONCONS_ASSERT(structure_stack_.size() > 0);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::object_kind);

        structure_stack_.pop_back();
        if (structure_stack_.back().is_key)
        {
            event_ = staj_event_type(staj_events::end_object, semantic_tag::none, staj_events::key_flag);
        }
        else 
        {
            event_ = staj_event_type(staj_events::end_object);
        }

        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure_stack_.back().is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(staj_events::begin_array, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(staj_events::begin_array, tag);
        }
        structure_stack_.emplace_back(json_structure_kind::array_kind);
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(std::size_t length,
        semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure_stack_.back().is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(staj_events::begin_array, length, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(staj_events::begin_array, tag);
        }
        structure_stack_.emplace_back(json_structure_kind::array_kind);
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) final
    {
        JSONCONS_ASSERT(structure_stack_.size() > 1);
        JSONCONS_ASSERT(structure_stack_.back().structure_kind == json_structure_kind::array_kind);
        structure_stack_.pop_back();

        if (structure_stack_.back().is_key)
        {
            event_ = staj_event_type(staj_events::end_array, semantic_tag::none, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(staj_events::end_array);
        }

        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& sv, semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(sv, staj_events::string_value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(sv, staj_events::string_value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
        semantic_tag tag, 
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, staj_events::byte_string_value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, staj_events::byte_string_value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
        uint64_t ext_tag, 
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, staj_events::byte_string_value, ext_tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, staj_events::byte_string_value, ext_tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value, 
        semantic_tag tag, 
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value, 
        semantic_tag tag, 
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_half(uint16_t value, 
        semantic_tag tag,   
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(half_arg, value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(half_arg, value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_double(double value, 
        semantic_tag tag,   
        const ser_context&,
        std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }

    JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag tag, const ser_context&, std::error_code&) final
    {
        auto& structure = structure_stack_.back();
        if (structure.structure_kind == json_structure_kind::object_kind)
        {
            structure.is_key = !structure.is_key;
        }
        if (structure.is_key)
        {
            event_ = staj_event_type(staj_events::null_value, tag, staj_events::key_flag);
        }
        else
        {
            event_ = staj_event_type(staj_events::null_value, tag);
        }
        JSONCONS_VISITOR_RETURN;
    }
};

using generic_staj_event_receiver = basic_generic_staj_event_receiver<char>;

} // namespace jsoncons

#endif // JSONCONS_GENERIC_STAJ_EVENT_RECEIVER_HPP
