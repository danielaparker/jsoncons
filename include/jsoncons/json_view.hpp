// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_VIEW_HPP
#define JSONCONS_JSON_VIEW_HPP

#include <functional> // std::reference_wrapper
#include <type_traits> // std::remove_const

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 

enum class json_view_storage_kind : uint8_t {json_ref,array,object};

template <typename Json,typename Allocator=std::allocator<char>>
class json_view
{
    using allocator_type = Allocator;
    using json_type = typename std::remove_const<Json>::type;
    using key_type = typename json_type::key_type;
    using policy_type = typename json_type::policy_type;
    using array = typename policy_type::template array<json_view>;
    using object = typename policy_type::template object<key_type,json_view>;

    struct common_storage
    {
        json_view_storage_kind storage_kind_;
    };

    struct json_ref_storage 
    {
        json_view_storage_kind storage_kind_;
        std::reference_wrapper<Json> ref_;

        json_ref_storage(Json& ref)
            : storage_kind_(json_view_storage_kind::json_ref),
              ref_(ref)
        {
        }
    };

    struct array_storage
    {
        using allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<array>;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;

       json_view_storage_kind storage_kind_;
        pointer ptr_;

        array_storage(pointer ptr)
            : storage_kind_(static_cast<uint8_t>(json_view_storage_kind::array)), ptr_(ptr)
        {
        }

        array_storage(const array_storage& other)
            : storage_kind_(other.storage_kind_), ptr_(other.ptr_)
        {
        }

        Allocator get_allocator() const
        {
            return ptr_->get_allocator();
        }
    };
    struct object_storage
    {
        using allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<object>;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;

        json_view_storage_kind storage_kind_;
        pointer ptr_;

        object_storage(pointer ptr)
            : storage_kind_(static_cast<uint8_t>(json_view_storage_kind::object)), ptr_(ptr)
        {
        }

        explicit object_storage(const object_storage& other)
            : storage_kind_(other.storage_kind_), ptr_(other.ptr_)
        {
        }

        Allocator get_allocator() const
        {
            JSONCONS_ASSERT(ptr_ != nullptr);
            return ptr_->get_allocator();
        }
    };
    union 
    {
        common_storage common_;
        json_ref_storage json_ref_;
        array_storage array_;
        object_storage object_;
    };
public:
    json_view(Json& j)
    {
        common_.storage_kind_ = json_view_storage_kind::json_ref;
        json_ref_ = json_ref_storage{j};
    }

    json_view(const json_view<Json>& jv)
    {
        common_.storage_kind_ = jv.common_.storage_kind_;
        switch (jv.common_.storage_kind_)
        {
            case json_view_storage_kind::json_ref:
                json_ref_ = jv.json_ref_;
                break;
            default:
                JSONCONS_UNREACHABLE();
        }
    }

    template <typename T,typename... Args>
    bool is(Args&&... args) const noexcept
    {
        switch (common_.storage_kind_)
        {
            case json_view_storage_kind::json_ref:
                return json_ref_.ref_.get().template is<T>(std::forward<Args>(args)...);
            default:
                JSONCONS_UNREACHABLE();
        }
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_VIEW_HPP
