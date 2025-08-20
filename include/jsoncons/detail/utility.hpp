/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_DETAIL_UTILITY_HPP    
#define JSONCONS_DETAIL_UTILITY_HPP    

#include <utility>
#include <jsoncons/config/compiler_support.hpp>
#include <iostream>

namespace jsoncons {
namespace detail {
    
#if (defined(JSONCONS_HAS_2017))

    using in_place_t = std::in_place_t;

#else
    
    struct in_place_t
    {
        explicit in_place_t() = default; 
    };

#endif

JSONCONS_INLINE_CONSTEXPR in_place_t in_place{};

template <typename Alloc>
struct allocator_delete  : public Alloc
{
    using allocator_type = Alloc;
    using alloc_traits = std::allocator_traits<Alloc>;
    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using value_type = typename std::allocator_traits<Alloc>::value_type;

    allocator_delete(const Alloc& alloc) noexcept
        : Alloc(alloc)
    {
    }

    allocator_delete(const allocator_delete&) noexcept = default;

    template <typename T>
    typename std::enable_if<std::is_convertible<T&,value_type&>::value>::type
    operator()(T* ptr) noexcept
    {
        using rebind = typename std::allocator_traits<allocator_type>:: template rebind_alloc<T>;

        std::cout << "Type of T: " << typeid(*ptr).name() << ", sizeof(T): " << sizeof(T) << std::endl;
        //ptr->~T();
        //rebind alloc(*this);
        //alloc.deallocate(ptr, 1);
        alloc_traits::destroy(*this, ptr);
        alloc_traits::deallocate(*this, ptr, 1);    }
};

} // namespace detail
} // namespace jsoncons

#endif // JSONCONS_DETAIL_UTILITY_HPP
