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
#include <jsoncons/json_array.hpp>
#include <jsoncons/json_object.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json.hpp>

namespace jsoncons { 

struct json_view_object_t
{
    explicit json_view_object_t() = default; 
};

JSONCONS_INLINE_CONSTEXPR json_view_object_t json_view_object{};

struct json_view_array_t
{
    explicit json_view_array_t() = default;
};

JSONCONS_INLINE_CONSTEXPR json_view_array_t json_view_array{};

enum class json_view_storage_kind : uint8_t {const_ref,array,object};

template <typename Json,typename Allocator=std::allocator<char>>
class json_view
{
public:
    using json_type = Json
    using allocator_type = Allocator;
    using storage_kind_type = json_view_storage_kind;
    using key_type = typename Json::key_type;
    using policy_type = typename Json::policy_type;
    using array = typename policy_type::template array<json_view>;
    using object = typename policy_type::template object<key_type,json_view>;
    using char_type = typename Json::char_type;
    using string_view_type = typename Json::string_view_type;

    using array_iterator = typename array_iterator_typedefs<policy_type,key_type,json_view<Json,Allocator>>::array_iterator_type;                    
    using const_array_iterator = typename array_iterator_typedefs<policy_type,key_type,json_view<Json,Allocator>>::const_array_iterator_type;
    using array_range_type = range<array_iterator, const_array_iterator>;
    using const_array_range_type = range<const_array_iterator, const_array_iterator>;

    using object_iterator = typename object_iterator_typedefs<policy_type,key_type,json_view<Json,Allocator>>::object_iterator_type;                    
    using const_object_iterator = typename object_iterator_typedefs<policy_type,key_type,json_view<Json,Allocator>>::const_object_iterator_type;
    using object_range_type = range<object_iterator, const_object_iterator>;
    using const_object_range_type = range<const_object_iterator, const_object_iterator>;
private:
    struct common_storage
    {
        json_view_storage_kind storage_kind_;
    };

    struct const_ref_storage 
    {
        json_view_storage_kind storage_kind_;
        const Json* ptr_;

        const_ref_storage(const Json& ref)
            : storage_kind_(json_view_storage_kind::const_ref),
              ptr_(std::addressof(ref))
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
            : storage_kind_(json_view_storage_kind::array), ptr_(ptr)
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
            : storage_kind_(json_view_storage_kind::object), ptr_(ptr)
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
        const_ref_storage ref_;
        array_storage array_;
        object_storage object_;
    };
public:
    json_view(const Json& j)
    {
        common_.storage_kind_ = json_view_storage_kind::const_ref;
        ref_ = const_ref_storage{j};
    }

    json_view(const json_view<Json>& jv)
    {
        common_.storage_kind_ = jv.common_.storage_kind_;
        switch (jv.common_.storage_kind_)
        {
            case json_view_storage_kind::const_ref:
                ref_ = jv.ref_;
                break;
            default:
                JSONCONS_UNREACHABLE();
        }
    }

    explicit json_view(json_view_object_t) 
    {
        auto ptr = create_object(Allocator{});
        construct<object_storage>(ptr);
    }

    json_view(json_view_object_t, const Allocator& alloc) 
    {
        auto ptr = create_object(alloc);
        construct<object_storage>(ptr);
    }

    explicit json_view(json_view_array_t) 
    {
        auto ptr = create_array(Allocator{});
        construct<array_storage>(ptr);
    }

    json_view(json_view_array_t, const Allocator& alloc) 
    {
        auto ptr = create_array(alloc);
        construct<array_storage>(ptr);
    }

    ~json_view() noexcept
    {
         destroy();
    }

    json_view& operator=(const json_view& other)
    {
        if (this != &other)
        {
            copy_assignment(other);
        }
        return *this;
    }

    json_view_storage_kind storage_kind() const
    {
        return common_.storage_kind_;
    }

    bool empty() const noexcept
    {
        switch (storage_kind())
        {
            case json_view_storage_kind::array:
                return cast<array_storage>().ptr_->empty();
            case json_view_storage_kind::object:
                return cast<object_storage>().ptr_->empty();
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().ptr_->empty();
            default:
                return false;
        }
    }

    std::size_t capacity() const
    {
        switch (storage_kind())
        {
            case json_view_storage_kind::array:
                return cast<array_storage>().ptr_->capacity();
            case json_view_storage_kind::object:
                return cast<object_storage>().ptr_->capacity();
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().ref.capacity();
            default:
                return 0;
        }
    }

    const_object_range_type object_range() const
    {
        switch (storage_kind())
        {
            case json_view_storage_kind::object:
                return const_object_range_type(const_object_iterator(cast<object_storage>().ptr_->begin()),
                                                    const_object_iterator(cast<object_storage>().ptr_->end()));
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().val_.object_range();
            default:
                JSONCONS_THROW(json_runtime_error<std::domain_error>("Not an object"));
        }
    }

    const_array_range_type array_range() const
    {
        switch (storage_kind())
        {
            case json_view_storage_kind::array:
                return const_array_range_type(cast<array_storage>().ptr_->begin(),
                    cast<array_storage>().ptr_->end());
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().ptr_->array_range();
            default:
                JSONCONS_THROW(json_runtime_error<std::domain_error>("Not an array"));
        }
    }

    template <typename T,typename... Args>
    bool is(Args&&... args) const noexcept
    {
        switch (common_.storage_kind_)
        {
            case json_view_storage_kind::const_ref:
                return ref_.ptr_->template is<T>(std::forward<Args>(args)...);
            default:
                JSONCONS_UNREACHABLE();
        }
    }

    bool is_object() const noexcept
    {
        switch (common_.storage_kind_)
        {
            case json_view_storage_kind::object:
                return true;
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().value().is_object();
            default:
                return false;
        }
    }

    bool is_array() const noexcept
    {
        switch (common_.storage_kind_)
        {
            case json_view_storage_kind::array:
                return true;
            case json_view_storage_kind::const_ref:
                return cast<const_ref_storage>().value().is_object();
            default:
                return false;
        }
    }

    void clear()
    {
        switch (storage_kind())
        {
            case json_view_storage_kind::array:
                cast<array_storage>().ptr_->clear();
                break;
            case json_view_storage_kind::object:
                cast<object_storage>().ptr_->clear();
                break;
            default:
                break;
        }
    }

private:

    void copy_assignment(const json_view& other)
    {
        if (other.storage_kind() == json_view_storage_kind::const_ref)
        {
            destroy();
            std::memcpy(static_cast<void*>(this), &other, sizeof(json_view));
        }
        else if (storage_kind() == other.storage_kind())
        {
            switch (other.storage_kind())
            {
                case json_view_storage_kind::array:
                    cast<array_storage>().ptr_ = other.cast<array_storage>().ptr_;
                    break;
                case json_view_storage_kind::object:
                    cast<object_storage>().ptr_ = other.cast<object_storage>().ptr_;
                    break;
                default:
                    JSONCONS_UNREACHABLE();
                    break;
            }
        }
        else if (storage_kind() == json_view_storage_kind::const_ref) // rhs is not trivial storage
        {
            destroy();
            uninitialized_copy(other);
        }
        else // lhs and rhs are not trivial storage
        {
            switch (storage_kind())
            {
                case json_view_storage_kind::array:
                    auto alloc = cast<array_storage>().get_allocator();
                    destroy();
                    uninitialized_copy_a(other, alloc);
                    break;
                case json_view_storage_kind::object:
                    auto alloc = cast<object_storage>().get_allocator();
                    destroy();
                    uninitialized_copy_a(other, alloc);
                    break;
                default:
                    break;
            }
        }
    }

    void uninitialized_copy_a(const json_view& other, const Allocator& alloc)
    {
        if (other.storage_kind() == json_view_storage_kind::const_ref)
        {
            std::memcpy(static_cast<void*>(this), &other, sizeof(json_view));
        }
        else
        {
            switch (other.storage_kind())
            {
                case json_view_storage_kind::array:
                {
                    auto ptr = create_array(alloc, other.cast<array_storage>().value());
                    construct<array_storage>(ptr, other.tag());
                    break;
                }
                case json_view_storage_kind::object:
                {
                    auto ptr = create_object(alloc, other.cast<object_storage>().value());
                    construct<object_storage>(ptr, other.tag());
                    break;
                }
                default:
                    JSONCONS_UNREACHABLE();
                    break;
            }
        }
    }

    void uninitialized_copy(const json_view& other)
    {
        if (other.storage_kind() == json_view_storage_kind::const_ref)
        {
            std::memcpy(static_cast<void*>(this), &other, sizeof(json_view));
        }
        else
        {
            switch (other.storage_kind())
            {
                case json_view_storage_kind::array:
                {
                    auto ptr = create_array(
                        std::allocator_traits<Allocator>::select_on_container_copy_construction(other.cast<array_storage>().get_allocator()), 
                        other.cast<array_storage>().value());
                    construct<array_storage>(ptr, other.tag());
                    break;
                }
                case json_view_storage_kind::object:
                {
                    auto ptr = create_object(
                        std::allocator_traits<Allocator>::select_on_container_copy_construction(other.cast<object_storage>().get_allocator()), 
                        other.cast<object_storage>().value());
                    construct<object_storage>(ptr, other.tag());
                    break;
                }
                default:
                    JSONCONS_UNREACHABLE();
                    break;
            }
        }
    }

    template <typename StorageType,typename... Args>
    void construct(Args&&... args)
    {
        ::new (&cast<StorageType>()) StorageType(std::forward<Args>(args)...);
    }

    void destroy()
    {
        switch (common_.storage_kind_)
        {
            case json_view_storage_kind::array:
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
            case json_view_storage_kind::object:
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

    const_ref_storage& cast(identity<const_ref_storage>) 
    {
        return ref_;
    }

    const const_ref_storage& cast(identity<const_ref_storage>) const
    {
        return ref_;
    }

    template <typename... Args>
    typename array_storage::pointer create_array(const allocator_type& alloc, Args&& ... args)
    {
        using stor_allocator_type = typename array_storage::allocator_type;
        stor_allocator_type stor_alloc(alloc);
        auto ptr = std::allocator_traits<stor_allocator_type>::allocate(stor_alloc, 1);
        JSONCONS_TRY
        {
            std::allocator_traits<stor_allocator_type>::construct(stor_alloc, ext_traits::to_plain_pointer(ptr), 
                std::forward<Args>(args)...);
        }
        JSONCONS_CATCH(...)
        {
            std::allocator_traits<stor_allocator_type>::deallocate(stor_alloc, ptr,1);
            JSONCONS_RETHROW;
        }
        return ptr;
    }

    template <typename... Args>
    typename object_storage::pointer create_object(const allocator_type& alloc, Args&& ... args)
    {
        using stor_allocator_type = typename object_storage::allocator_type;
        stor_allocator_type stor_alloc(alloc);
        auto ptr = std::allocator_traits<stor_allocator_type>::allocate(stor_alloc, 1);
        JSONCONS_TRY
        {
            std::allocator_traits<stor_allocator_type>::construct(stor_alloc, ext_traits::to_plain_pointer(ptr), 
                std::forward<Args>(args)...);
        }
        JSONCONS_CATCH(...)
        {
            std::allocator_traits<stor_allocator_type>::deallocate(stor_alloc, ptr,1);
            JSONCONS_RETHROW;
        }
        return ptr;
    }
};

} // namespace jsoncons

#endif // JSONCONS_JSON_VIEW_HPP
