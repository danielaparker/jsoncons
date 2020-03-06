// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_HEAP_ONLY_STRING_HPP
#define JSONCONS_DETAIL_HEAP_ONLY_STRING_HPP

#include <stdexcept>
#include <string>
#include <exception>
#include <ostream>
#include <cstring> // std::memcpy
#include <memory> // std::allocator
#include <jsoncons/config/compiler_support.hpp>

namespace jsoncons { namespace detail {

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
template <class Allocator>
class heap_only_string_base
{
    Allocator alloc_;
public:
    Allocator& get_allocator() 
    {
        return alloc_;
    }

    const Allocator& get_allocator() const
    {
        return alloc_;
    }
protected:
    heap_only_string_base(const Allocator& alloc)
        : alloc_(alloc)
    {
    }

    ~heap_only_string_base() {}
};

template <class CharT,class Allocator>
class heap_only_string_factory;

template <class CharT, class Allocator>
class heap_only_string_wrapper;

template <class CharT, class Allocator>
class heap_only_string : public heap_only_string_base<Allocator>
{
    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<CharT> allocator_type;  
    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::pointer pointer;

    friend class heap_only_string_factory<CharT, Allocator>;
    friend class heap_only_string_wrapper<CharT, Allocator>;

	pointer p_;
	size_t length_;
public:
    typedef CharT char_type;

    ~heap_only_string() {}

    const char_type* c_str() const { return to_plain_pointer(p_); }
    const char_type* data() const { return to_plain_pointer(p_); }
    std::size_t length() const { return length_; }

    using heap_only_string_base<Allocator>::get_allocator;


    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const heap_only_string& s)
    {
        os.write(s.data(),s.length());
        return os;
    }
private:
    heap_only_string()
        : heap_only_string_base<Allocator>(Allocator()), p_(nullptr), length_(0)
    {

    }
    heap_only_string(const Allocator& alloc)
        : heap_only_string_base<Allocator>(alloc), p_(nullptr), length_(0)
    {

    }

    heap_only_string(const heap_only_string&) = delete;
    heap_only_string& operator=(const heap_only_string&) = delete;

};

template <class CharT,class Allocator>
class heap_only_string_wrapper
{
    typedef CharT char_type;
    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> byte_allocator_type;  
    typedef typename std::allocator_traits<byte_allocator_type>::pointer byte_pointer;

    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<heap_only_string<CharT,Allocator>> string_allocator_type;  
    typedef typename std::allocator_traits<string_allocator_type>::pointer string_pointer;

    typedef heap_only_string<CharT,Allocator> string_type;

    struct string_storage
    {
        string_type data;
        char_type c[1];
    };
    typedef typename std::aligned_storage<sizeof(string_storage), alignof(string_storage)>::type storage_kind;

    string_pointer ptr_;
public:
    heap_only_string_wrapper() = default;

    heap_only_string_wrapper(string_pointer ptr)
        : ptr_(ptr)
    {
    }

    heap_only_string_wrapper(const char_type* data, std::size_t length, const Allocator& a) 
    {
        ptr_ = create(data,length,a);
    }

    heap_only_string_wrapper(const heap_only_string_wrapper& val) 
    {
        ptr_ = create(val.data(),val.length(),val.get_allocator());
    }

    heap_only_string_wrapper(const heap_only_string_wrapper& val, const Allocator& a) 
    {
        ptr_ = create(val.data(),val.length(),a);
    }

    ~heap_only_string_wrapper()
    {
        if (ptr_ != nullptr)
        {
            destroy(ptr_);
        }
    }

    void swap(heap_only_string_wrapper& other) noexcept
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
        return sizeof(storage_kind) + n;
    }

    static string_pointer create(const char_type* s, std::size_t length, const Allocator& alloc)
    {
        std::size_t mem_size = aligned_size(length*sizeof(char_type));

        byte_allocator_type byte_alloc(alloc);
        byte_pointer ptr = byte_alloc.allocate(mem_size);

        char* storage = to_plain_pointer(ptr);
        string_type* ps = new(storage)heap_only_string<char_type,Allocator>(byte_alloc);

        auto psa = launder_cast<string_storage*>(storage); 

        CharT* p = new(&psa->c)char_type[length + 1];
        std::memcpy(p, s, length*sizeof(char_type));
        p[length] = 0;
        ps->p_ = std::pointer_traits<typename string_type::pointer>::pointer_to(*p);
        ps->length_ = length;
        return std::pointer_traits<string_pointer>::pointer_to(*ps);
    }

    static void destroy(string_pointer ptr)
    {
        string_type* rawp = to_plain_pointer(ptr);

        char* p = launder_cast<char*>(rawp);

        std::size_t mem_size = aligned_size(ptr->length_*sizeof(char_type));
        byte_allocator_type byte_alloc(ptr->get_allocator());
        byte_alloc.deallocate(p,mem_size);
    }
};


}}

#endif
