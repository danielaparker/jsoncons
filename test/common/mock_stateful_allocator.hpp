// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#ifndef MOCK_STATEFUL_ALLOCATOR
#define MOCK_STATEFUL_ALLOCATOR

#include <algorithm>
#include <memory>
#include <list>
#include <type_traits>
#include <utility>

template <typename T,typename PropagateOnCCA=std::false_type,typename PropagateOnCMA=std::true_type,typename PropagateOnCS=std::true_type>
class mock_stateful_allocator
{
    std::allocator<T> impl_;
    int id_;
public:
    using value_type = T;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_copy_assignment = PropagateOnCCA;
    using propagate_on_container_move_assignment = PropagateOnCMA;
    using propagate_on_container_swap = PropagateOnCS;
    using is_always_equal = std::false_type;

    mock_stateful_allocator() = delete;

    mock_stateful_allocator(int id) noexcept
        : impl_(), id_(id)
    {
    }

    mock_stateful_allocator(const mock_stateful_allocator<T>& other) noexcept
        : impl_(), id_(other.id_)
    {
    }

    template <typename U,typename V, typename W, typename X>
    friend class mock_stateful_allocator;

    template <typename U, typename V, typename W, typename X>
    mock_stateful_allocator(const mock_stateful_allocator<U,V,W,X>& other) noexcept
        : impl_(), id_(other.id_)
    {
    }

    mock_stateful_allocator& operator = (const mock_stateful_allocator& other) = default;

    T* allocate(size_type n) 
    {
        return impl_.allocate(n);
    }

    void deallocate(T* ptr, size_type n) 
    {
        impl_.deallocate(ptr, n);
    }

    friend bool operator==(const mock_stateful_allocator& lhs, const mock_stateful_allocator& rhs) noexcept
    {
        return lhs.id_ == rhs.id_;
    }

    friend bool operator!=(const mock_stateful_allocator& lhs, const mock_stateful_allocator& rhs) noexcept
    {
        return lhs.id_ != rhs.id_;
    }
};

template <typename T>
using non_propagating_allocator = mock_stateful_allocator<T,std::false_type,std::false_type,std::false_type>;

template <typename T>
using propagating_allocator = mock_stateful_allocator<T,std::true_type,std::true_type,std::true_type>;

#endif
