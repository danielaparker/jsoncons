// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_REF_HPP
#define JSONCONS_JSON_REF_HPP

#include <functional> // std::reference_wrapper
#include <type_traits> // std::remove_const

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 

enum class json_ref_storage_kind : uint8_t {json_ref,array,object};

template <typename Json,typename Allocator=std::allocator<char>>
class json_ref
{
    using allocator_type = Allocator;
    using json_type = typename std::remove_const<Json>::type;
    using key_type = typename json_type::key_type;
    using policy_type = typename json_type::policy_type;
    using array = typename policy_type::template array<json_ref>;
    using object = typename policy_type::template object<key_type,json_ref>;

    struct common_storage
    {
        json_ref_storage_kind storage_kind_;
    };

    struct ref_storage 
    {
        json_ref_storage_kind storage_kind_;
        std::reference_wrapper<Json> ref_;

        ref_storage(Json& ref)
            : storage_kind_(json_ref_storage_kind::json_ref),
              ref_(ref)
        {
        }
    };

    struct array_storage
    {
        using allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<array>;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;

       json_ref_storage_kind storage_kind_;
        pointer ptr_;

        array_storage(pointer ptr)
            : storage_kind_(json_ref_storage_kind::array), ptr_(ptr)
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

        json_ref_storage_kind storage_kind_;
        pointer ptr_;

        object_storage(pointer ptr)
            : storage_kind_(json_ref_storage_kind::object), ptr_(ptr)
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
        ref_storage ref_;
        array_storage array_;
        object_storage object_;
    };
public:
    json_ref(Json& j)
    {
        common_.storage_kind_ = json_ref_storage_kind::json_ref;
        ref_ = ref_storage{j};
    }

    json_ref(const json_ref<Json>& jv)
    {
        common_.storage_kind_ = jv.common_.storage_kind_;
        switch (jv.common_.storage_kind_)
        {
            case json_ref_storage_kind::json_ref:
                ref_ = jv.ref_;
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
            case json_ref_storage_kind::json_ref:
                return ref_.ref_.get().template is<T>(std::forward<Args>(args)...);
            default:
                JSONCONS_UNREACHABLE();
        }
    }
private:

    void destroy()
    {
        switch (common_.storage_kind_)
        {
            case json_ref_storage_kind::array:
            {
                if (cast<array_storage>().ptr_ != nullptr)
                {
                    auto& stor = cast<array_storage>();
                    typename array_storage::allocator_type alloc{stor.ptr_->get_allocator()};
                    std::allocator_traits<typename array_storage::allocator_type>::destroy(alloc, ext_traits::to_plain_pointer(stor.ptr_));
                    std::allocator_traits<typename array_storage::allocator_type>::deallocate(alloc, stor.ptr_,1);
                }
                break;
            }
            case json_ref_storage_kind::object:
            {
                if (cast<object_storage>().ptr_ != nullptr)
                {
                    auto& stor = cast<object_storage>();
                    typename object_storage::allocator_type alloc{stor.ptr_->get_allocator()};
                    std::allocator_traits<typename object_storage::allocator_type>::destroy(alloc, ext_traits::to_plain_pointer(stor.ptr_));
                    std::allocator_traits<typename object_storage::allocator_type>::deallocate(alloc, stor.ptr_,1);
                }
                break;
            }
            default:
                break;
        }
    }

    template <typename T>
    struct identity { using type = T*; };

    template <typename T> 
    T& cast()
    {
        return cast(identity<T>());
    }

    template <typename T> 
    const T& cast() const
    {
        return cast(identity<T>());
    }

    object_storage& cast(identity<object_storage>)
    {
        return object_;
    }

    const object_storage& cast(identity<object_storage>) const
    {
        return object_;
    }

    array_storage& cast(identity<array_storage>)
    {
        return array_;
    }

    const array_storage& cast(identity<array_storage>) const
    {
        return array_;
    }

    ref_storage& cast(identity<ref_storage>) 
    {
        return ref_;
    }

    const ref_storage& cast(identity<ref_storage>) const
    {
        return ref_;
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_REF_HPP
