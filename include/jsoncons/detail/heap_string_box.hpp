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

    // heap_string_box

    template <class CharT,class Allocator>
    class heap_string_box
    {
    public:
        using char_type = CharT;
    private:
        struct heap_string_base
        {
            Allocator alloc_;
        
            Allocator& get_allocator() 
            {
                return alloc_;
            }

            const Allocator& get_allocator() const
            {
                return alloc_;
            }

            heap_string_base(const Allocator& alloc)
                : alloc_(alloc)
            {
            }

            ~heap_string_base() noexcept = default;
        };

        struct heap_string : public heap_string_base
        {
            using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<CharT>;  
            using allocator_traits_type = std::allocator_traits<allocator_type>;
            using pointer = typename allocator_traits_type::pointer;

            pointer p_;
            std::size_t length_;

            ~heap_string() noexcept = default; 

            const char_type* c_str() const { return traits_extension::to_plain_pointer(p_); }
            const char_type* data() const { return traits_extension::to_plain_pointer(p_); }
            std::size_t length() const { return length_; }
        
            heap_string(const Allocator& alloc)
                : heap_string_base(alloc), p_(nullptr), length_(0)
            {

            }

            heap_string(const heap_string&) = delete;
            heap_string& operator=(const heap_string&) = delete;

        };

        using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;  
        using byte_pointer = typename std::allocator_traits<byte_allocator_type>::pointer;

        using heap_string_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<heap_string>;  
        using heap_string_pointer = typename std::allocator_traits<heap_string_allocator_type>::pointer;

        struct storage_t
        {
            heap_string data;
            char_type c[1];
        };
        typedef typename jsoncons_aligned_storage<sizeof(storage_t), alignof(storage_t)>::type json_storage_kind;

        heap_string_pointer ptr_;
    public:
        heap_string_box() = default;

        heap_string_box(heap_string_pointer ptr)
            : ptr_(ptr)
        {
        }

        heap_string_box(const char_type* data, std::size_t length, const Allocator& a) 
        {
            ptr_ = create(data,length,a);
        }

        heap_string_box(const heap_string_box& val) 
        {
            ptr_ = create(val.data(),val.length(),val.get_allocator());
        }

        heap_string_box(const heap_string_box& val, const Allocator& a) 
        {
            ptr_ = create(val.data(),val.length(),a);
        }

        ~heap_string_box() noexcept
        {
            if (ptr_ != nullptr)
            {
                destroy(ptr_);
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

        Allocator get_allocator() const
        {
            return ptr_->get_allocator();
        }
    private:
        static size_t aligned_size(std::size_t n)
        {
            return sizeof(json_storage_kind) + n;
        }

        static heap_string_pointer create(const char_type* s, std::size_t length, const Allocator& alloc)
        {
            std::size_t mem_size = aligned_size(length*sizeof(char_type));

            byte_allocator_type byte_alloc(alloc);
            byte_pointer ptr = byte_alloc.allocate(mem_size);

            char* storage = traits_extension::to_plain_pointer(ptr);
            heap_string* ps = new(storage)heap_string(byte_alloc);

            auto psa = launder_cast<storage_t*>(storage); 

            CharT* p = new(&psa->c)char_type[length + 1];
            std::memcpy(p, s, length*sizeof(char_type));
            p[length] = 0;
            ps->p_ = std::pointer_traits<typename heap_string::pointer>::pointer_to(*p);
            ps->length_ = length;
            return std::pointer_traits<heap_string_pointer>::pointer_to(*ps);
        }

        static void destroy(heap_string_pointer ptr)
        {
            heap_string* rawp = traits_extension::to_plain_pointer(ptr);

            char* p = launder_cast<char*>(rawp);

            std::size_t mem_size = aligned_size(ptr->length_*sizeof(char_type));
            byte_allocator_type byte_alloc(ptr->get_allocator());
            byte_alloc.deallocate(p,mem_size);
        }
    };

    // tagged_heap_string_box

    template <class CharT,class Allocator>
    class tagged_heap_string_box
    {
    public:
        using char_type = CharT;
    private:
        struct heap_string_base
        {
            Allocator alloc_;

            Allocator& get_allocator() 
            {
                return alloc_;
            }

            const Allocator& get_allocator() const
            {
                return alloc_;
            }

            heap_string_base(const Allocator& alloc)
                : alloc_(alloc)
            {
            }

            ~heap_string_base() noexcept = default;
        };

        struct heap_string : public heap_string_base
        {
            using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<CharT>;  
            using allocator_traits_type = std::allocator_traits<allocator_type>;
            using pointer = typename allocator_traits_type::pointer;

            pointer p_;
            std::size_t length_;
            uint64_t tag_;

            ~heap_string() noexcept = default; 

            const char_type* c_str() const { return traits_extension::to_plain_pointer(p_); }
            const char_type* data() const { return traits_extension::to_plain_pointer(p_); }
            std::size_t length() const { return length_; }
            uint64_t tag() const { return tag_; }

            heap_string(uint64_t tag, const Allocator& alloc)
                : heap_string_base(alloc), p_(nullptr), length_(0), tag_(tag)
            {

            }

            heap_string(const heap_string&) = delete;
            heap_string& operator=(const heap_string&) = delete;

        };

        using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;  
        using byte_pointer = typename std::allocator_traits<byte_allocator_type>::pointer;

        using heap_string_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<heap_string>;  
        using heap_string_pointer = typename std::allocator_traits<heap_string_allocator_type>::pointer;

        struct storage_t
        {
            heap_string data;
            char_type c[1];
        };
        typedef typename jsoncons_aligned_storage<sizeof(storage_t), alignof(storage_t)>::type json_storage_kind;

        heap_string_pointer ptr_;
    public:
        tagged_heap_string_box() = default;

        tagged_heap_string_box(heap_string_pointer ptr)
            : ptr_(ptr)
        {
        }

        tagged_heap_string_box(const char_type* data, std::size_t length, uint64_t tag, const Allocator& alloc) 
        {
            ptr_ = create(data, length, tag, alloc);
        }

        tagged_heap_string_box(const tagged_heap_string_box& val) 
        {
            ptr_ = create(val.data(), val.length(), val.tag(), val.get_allocator());
        }

        tagged_heap_string_box(const tagged_heap_string_box& val, const Allocator& alloc) 
        {
            ptr_ = create(val.data(), val.length(), val.tag(), alloc);
        }

        ~tagged_heap_string_box() noexcept
        {
            if (ptr_ != nullptr)
            {
                destroy(ptr_);
            }
        }

        void swap(tagged_heap_string_box& other) noexcept
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

        uint64_t tag() const
        {
            return ptr_->tag();
        }

        Allocator get_allocator() const
        {
            return ptr_->get_allocator();
        }
    private:
        static size_t aligned_size(std::size_t n)
        {
            return sizeof(json_storage_kind) + n;
        }

        static heap_string_pointer create(const char_type* s, std::size_t length, uint64_t tag, const Allocator& alloc)
        {
            std::size_t mem_size = aligned_size(length*sizeof(char_type));

            byte_allocator_type byte_alloc(alloc);
            byte_pointer ptr = byte_alloc.allocate(mem_size);

            char* storage = traits_extension::to_plain_pointer(ptr);
            heap_string* ps = new(storage)heap_string(tag, byte_alloc);

            auto psa = launder_cast<storage_t*>(storage); 

            CharT* p = new(&psa->c)char_type[length + 1];
            std::memcpy(p, s, length*sizeof(char_type));
            p[length] = 0;
            ps->p_ = std::pointer_traits<typename heap_string::pointer>::pointer_to(*p);
            ps->length_ = length;
            return std::pointer_traits<heap_string_pointer>::pointer_to(*ps);
        }

        static void destroy(heap_string_pointer ptr)
        {
            heap_string* rawp = traits_extension::to_plain_pointer(ptr);

            char* p = launder_cast<char*>(rawp);

            std::size_t mem_size = aligned_size(ptr->length_*sizeof(char_type));
            byte_allocator_type byte_alloc(ptr->get_allocator());
            byte_alloc.deallocate(p,mem_size);
        }
    };

} // namespace detail
} // namespace jsoncons

#endif
