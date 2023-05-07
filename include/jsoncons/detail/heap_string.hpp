// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_HEAP_STRING_BOX_HPP
#define JSONCONS_DETAIL_HEAP_STRING_BOX_HPP

#include <stdexcept>
#include <string>
#include <exception>
#include <ostream>
#include <cstring> // std::memcpy
#include <memory> // std::allocator
#include <jsoncons/config/compiler_support.hpp>

namespace jsoncons { 
namespace detail {

    template <class Extra,class Allocator>
    struct heap_string_base
    {
        Extra extra_;
        Allocator alloc_;
    
        Allocator& get_allocator() 
        {
            return alloc_;
        }

        const Allocator& get_allocator() const
        {
            return alloc_;
        }

        heap_string_base(const Extra& extra, const Allocator& alloc)
            : extra_(extra), alloc_(alloc)
        {
        }

        ~heap_string_base() noexcept = default;
    };

    template <class CharT, class Extra, class Allocator>
    struct heap_string : public heap_string_base<Extra,Allocator>
    {
        using char_type = CharT;
        using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<CharT>;  
        using allocator_traits_type = std::allocator_traits<allocator_type>;
        using pointer = typename allocator_traits_type::pointer;

        pointer p_;
        std::size_t length_;

        ~heap_string() noexcept = default; 

        const char_type* c_str() const { return extension_traits::to_plain_pointer(p_); }
        const char_type* data() const { return extension_traits::to_plain_pointer(p_); }
        std::size_t length() const { return length_; }
        Extra extra() const { return this->extra_; }

        heap_string(Extra extra, const Allocator& alloc)
            : heap_string_base<Extra,Allocator>(extra, alloc), p_(nullptr), length_(0)
        {
        }

        heap_string(const heap_string&) = delete;
        heap_string& operator=(const heap_string&) = delete;

    };

    template<std::size_t Len, std::size_t Align>
    struct jsoncons_aligned_storage
    {
        struct type
        {
            alignas(Align) unsigned char data[Len];
        };
    };

    // From boost 1_71
    template <class T, class U>
    T launder_cast(U* u)
    {
    #if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606
        return std::launder(reinterpret_cast<T>(u));
    #elif defined(__GNUC__) &&  (__GNUC__ * 100 + __GNUC_MINOR__) > 800
        return __builtin_launder(reinterpret_cast<T>(u));
    #else
        return reinterpret_cast<T>(u);
    #endif
    }

    // heap_string_factory

    template <class CharT,class Extra,class Allocator>
    class heap_string_factory
    {
    public:
        using char_type = CharT;
        using heap_string_type = heap_string<CharT,Extra,Allocator>;
    private:

        using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;  
        using byte_pointer = typename std::allocator_traits<byte_allocator_type>::pointer;

        using heap_string_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<heap_string_type>;  
    public:
        using pointer = typename std::allocator_traits<heap_string_allocator_type>::pointer;

        struct storage_t
        {
            heap_string_type data;
            char_type c[1];
        };
        typedef typename jsoncons_aligned_storage<sizeof(storage_t), alignof(storage_t)>::type json_storage_kind;

        static size_t aligned_size(std::size_t n)
        {
            return sizeof(json_storage_kind) + n;
        }

    public:

        static pointer create(const char_type* s, std::size_t length, Extra extra, const Allocator& alloc)
        {
            std::size_t mem_size = aligned_size(length*sizeof(char_type));

            byte_allocator_type byte_alloc(alloc);
            byte_pointer ptr = byte_alloc.allocate(mem_size);

            char* storage = extension_traits::to_plain_pointer(ptr);
            heap_string_type* ps = new(storage)heap_string_type(extra, byte_alloc);

            auto psa = launder_cast<storage_t*>(storage); 

            CharT* p = new(&psa->c)char_type[length + 1];
            std::memcpy(p, s, length*sizeof(char_type));
            p[length] = 0;
            ps->p_ = std::pointer_traits<typename heap_string_type::pointer>::pointer_to(*p);
            ps->length_ = length;
            return std::pointer_traits<pointer>::pointer_to(*ps);
        }

        static void destroy(pointer ptr)
        {
            if (ptr != nullptr)
            {
                heap_string_type* rawp = extension_traits::to_plain_pointer(ptr);

                char* p = launder_cast<char*>(rawp);

                std::size_t mem_size = aligned_size(ptr->length_*sizeof(char_type));
                byte_allocator_type byte_alloc(ptr->get_allocator());
                byte_alloc.deallocate(p,mem_size);
            }
        }
    };

    // heap_string_box

    template <class CharT,class Extra,class Allocator>
    class heap_string_box
    {
        using heap_string_factory_type = heap_string_factory<CharT,Extra,Allocator>;
    public:
        using char_type = CharT;
        using heap_string_type = heap_string<CharT,Extra,Allocator>;
    private:

        using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;  
        using byte_pointer = typename std::allocator_traits<byte_allocator_type>::pointer;

        using heap_string_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<heap_string_type>;  
        using heap_string_pointer = typename std::allocator_traits<heap_string_allocator_type>::pointer;

        struct storage_t
        {
            heap_string_type data;
            char_type c[1];
        };
        typedef typename jsoncons_aligned_storage<sizeof(storage_t), alignof(storage_t)>::type json_storage_kind;

        heap_string_pointer ptr_;
    public:
        //heap_string_box() = default;

        heap_string_box(heap_string_pointer ptr)
            : ptr_(ptr)
        {
        }

        heap_string_box(const char_type* data, std::size_t length, Extra extra, const Allocator& alloc) 
        {
            ptr_ = heap_string_factory_type::create(data, length, extra, alloc);
        }

        heap_string_box(const heap_string_box& other) 
        {
            ptr_ = heap_string_factory_type::create(other.data(), other.length(), other.extra(), other.get_allocator());
        }

        heap_string_box(const heap_string_box& other, const Allocator& alloc) 
        {
            ptr_ = heap_string_factory_type::create(other.data(), other.length(), other.extra(), alloc);
        }

        heap_string_box(heap_string_box&& other)
            : ptr_(nullptr)
        {
            std::swap(ptr_,other.ptr_);
        }

        heap_string_box(heap_string_box&& other, const Allocator& alloc)
            : ptr_(nullptr)
        {
            if (alloc == other.get_allocator())
            {
                std::swap(ptr_,other.ptr_);
            }
            else
            {
                ptr_ = heap_string_factory_type::create(other.data(), other.length(), other.extra(), alloc);
            }
        }

        heap_string_box& operator=(const heap_string_box& other)
        {
            if (ptr_ != nullptr)
            {
                heap_string_factory_type::destroy(ptr_);
            }
            ptr_ = heap_string_factory_type::create(other.data(), other.length(), other.extra(), other.get_allocator());
            return *this;
        }

        template <class U = Allocator>
        heap_string_box& operator=(heap_string_box&& other)
        {
            move_assignment(std::integral_constant<bool,std::allocator_traits<U>::propagate_on_container_move_assignment::value>(), std::move(other));
            return *this;
        }

        ~heap_string_box() noexcept
        {
            if (ptr_ != nullptr)
            {
                heap_string_factory_type::destroy(ptr_);
            }
        }

        void swap(heap_string_box& other) noexcept
        {
            std::swap(ptr_,other.ptr_);
        }

        const char_type* data() const
        {
            return ptr_->data();
        }

        const char_type* c_str() const
        {
            return ptr_->c_str();
        }

        std::size_t length() const
        {
            return ptr_->length();
        }

        Extra extra() const
        {
            return ptr_->extra();
        }

        Allocator get_allocator() const
        {
            return ptr_->get_allocator();
        }
    private:
        void move_assignment(std::true_type, heap_string_box&& other)
        {
            if (ptr_ != nullptr)
            {
                heap_string_factory_type::destroy(ptr_);
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        void move_assignment(std::false_type, heap_string_box&& other)
        {
            if (get_allocator() == other.get_allocator())
            {
                if (ptr_ != nullptr)
                {
                    heap_string_factory_type::destroy(ptr_);
                }
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            else
            {
                if (ptr_ != nullptr)
                {
                    heap_string_factory_type::destroy(ptr_);
                }
                ptr_ = heap_string_factory_type::create(other.data(), other.length(), other.extra(), other.get_allocator());
            }
        }
    };

} // namespace detail
} // namespace jsoncons

#endif
