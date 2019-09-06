// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <catch/catch.hpp>

using namespace jsoncons;

// http://coliru.stacked-crooked.com/a/cfd0c5c5021596ad

template <typename T>
class FreelistAllocator {
    union node {
        node* next;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    };

    node* list = nullptr;

    void clear() noexcept {
        auto p = list;
        while (p) {
            auto tmp = p;
            p = p->next;
            delete tmp;
        }
        list = nullptr;
    }

public:
    using value_type = T;
    using size_type = std::size_t;
    using propagate_on_container_move_assignment = std::true_type;

    FreelistAllocator() noexcept = default;
    FreelistAllocator(const FreelistAllocator&) noexcept {}
    template <typename U>
    FreelistAllocator(const FreelistAllocator<U>&) noexcept {}
    FreelistAllocator(FreelistAllocator&& other) noexcept :  list(other.list) {
        other.list = nullptr;
    }

    FreelistAllocator& operator = (const FreelistAllocator&) noexcept {
        // noop
        return *this;
    }

    FreelistAllocator& operator = (FreelistAllocator&& other) noexcept {
        clear();
        list = other.list;
        other.list = nullptr;
        return *this;
    }

    ~FreelistAllocator() noexcept { clear(); }

    T* allocate(size_type n) {
        std::cout << "Allocate(" << n << ") from ";
        if (n == 1) {
            auto ptr = list;
            if (ptr) {
                std::cout << "freelist\n";
                list = list->next;
            } else {
                std::cout << "new node\n";
                ptr = new node;
            }
            return reinterpret_cast<T*>(ptr);
        }

        std::cout << "::operator new\n";
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* ptr, size_type n) noexcept {
        std::cout << "Deallocate(" << static_cast<void*>(ptr) << ", " << n << ") to ";
        if (n == 1) {
            std::cout << "freelist\n";
            auto node_ptr = reinterpret_cast<node*>(ptr);
            node_ptr->next = list;
            list = node_ptr;
        } else {
            std::cout << "::operator delete\n";
            ::operator delete(ptr);
        }
    }
};

template <typename T, typename U>
inline bool operator == (const FreelistAllocator<T>&, const FreelistAllocator<U>&) {
    return true;
}

template <typename T, typename U>
inline bool operator != (const FreelistAllocator<T>&, const FreelistAllocator<U>&) {
    return false;
}

/*
class pool 
{ 
private: 
    struct node_type 
    { 
        char* memory_ptr_; 
        node_type* next_ptr_; 
    }; 
    struct node_typeA
    {
        node_type data;
        char c[1];
    };
    typedef std::aligned_storage<sizeof(node_typeA), alignof(node_typeA)>::type storage_type;

    size_t offset_, size_; 
    node_type* head_; 
    node_type* curr_; 

    node_type* add_node()
    {
        size_t mem_size = sizeof(storage_type) + size_ - 1;
        node_type* storage = reinterpret_cast<node_type*>(alloc(mem_size)); 
        node_type* p = new(storage)node_type();
        auto pa = reinterpret_cast<node_typeA*>(storage);
        p->memory_ptr_ = new(&pa->c)char[size_];
        p->next_ptr_ = nullptr;
        return storage; 
    }

    void* alloc(size_t n) {return malloc(n);} 
    void dealloc(void* storage) {free(storage);} 
public: 
    pool(size_t size)
        : offset_(0), size_(size), allocate_count_(0), deallocate_count_(0), construct_count_(0), destroy_count_(0)  
    { 
        head_ = curr_ = add_node(); 
    } 
    ~pool()
    { 
        while (head_) 
        { 
            node_type* curr2_ = head_->next_ptr_; 
            dealloc(head_); 
            head_ = curr2_; 
        } 
    } 
    void* allocate(size_t n)
    { 
        void *pv; 
        if (n > (size_ - offset_)) 
        { 
            if (size_ < n) 
            {
                size_ = n;
            }
            curr_->next_ptr_ = add_node(); 
            curr_ = curr_->next_ptr_; 
            offset_ = 0; 
        } 
        pv = reinterpret_cast<void *>(curr_->memory_ptr_ + offset_); 

        size_t mem_size = sizeof(storage_type) + n - 1;

        offset_ += mem_size; 
        return pv; 
    }

    size_t allocate_count_;
    size_t deallocate_count_;
    size_t construct_count_;
    size_t destroy_count_;
}; 

template<class T> 
class pool_allocator 
{ 
public: 
    typedef size_t size_type; 
    typedef ptrdiff_t difference_type; 
    typedef T* pointer; 
    typedef const T* const_pointer; 
    typedef T& reference; 
    typedef const T& const_reference; 
    typedef T value_type; 
    //template<typename U> 
    //struct rebind 
    //{
    //    typedef pool_allocator<U> other;
    //}; 
    typedef std::true_type propagate_on_container_move_assignment;

    pool_allocator(pool* pp) throw() 
        : pool_ptr_(pp)
    {
    } 
    pool_allocator(const pool_allocator& s) throw() 
        : pool_ptr_(s.pool_ptr_)
    {
    } 
    template<typename U> 
    pool_allocator(const pool_allocator<U> &s) throw() 
        : pool_ptr_(s.pool_ptr_) 
    {
    } 
    ~pool_allocator() noexcept
    {
    } 
    pointer address(reference x) const 
    {
        return &x;
    } 
    const_pointer address(const_reference x) const 
    {
        return &x;
    } 
    pointer allocate(size_type n, const void* = 0) 
    {
        ++pool_ptr_->allocate_count_;
        return static_cast<T*>(pool_ptr_->allocate(n * sizeof(T)));
    } 
    void deallocate(pointer, size_type) 
    {
        ++pool_ptr_->deallocate_count_;
    }
    size_type max_size() const throw() 
    {
        return size_t(-1) / sizeof(T);
    } 
    template <typename... Args>
    void construct(pointer p, Args&&... args)
    {
        ::new(p) T(std::forward<Args>(args)...);
        ++pool_ptr_->construct_count_;
    }
    void destroy(pointer p) 
    {
        (void)p;
        p->~T();
        ++pool_ptr_->destroy_count_;
    } 
    pool* pool_ptr_; 
}; 

template<class T> 
bool operator==(const pool_allocator<T> &s0, const pool_allocator<T> &s1) 
{
    return s0.pool_ptr_ == s1.pool_ptr_;
} 
template<class T> 
bool operator!=(const pool_allocator<T> &s0, const pool_allocator<T> &s1) 
{
    return s0.pool_ptr_ != s1.pool_ptr_;
}
*/

#if !defined(__GNUC__) 
// gcc 4.8 basic_string doesn't satisfy C++11 allocator requirements
TEST_CASE("test_string_allocation")
{

    FreelistAllocator<char> allocator; 

    typedef basic_json<char,sorted_policy,FreelistAllocator<char>> myjson;

    SECTION("construct")
    {
        {
            myjson j("String too long for short string", allocator);
        }
        //std::cout << "Allocate count = " << a_pool.allocate_count_ 
        //          << ", construct count = " << a_pool.construct_count_ 
        //          << ", destroy count = " << a_pool.destroy_count_ 
        //          << ", deallocate count = " << a_pool.deallocate_count_ << std::endl;
    }

    SECTION("construct")
    {
        {
            myjson j("String too long for short string", allocator);
        }
        //CHECK(a_pool.allocate_count_ == a_pool.deallocate_count_);
       // CHECK(a_pool.construct_count_ == a_pool.destroy_count_);
    }

    SECTION("parse")
    {
        FreelistAllocator<char> allocator2; 

        std::string s = "String too long for short string";
        std::string input = "\"" + s + "\"";

        json_decoder<myjson,FreelistAllocator<char>> decoder(allocator, allocator2);
        try
        {
            json_reader reader(input,decoder);
            reader.read_next();
        }
        catch (const std::exception&)
        {
        }
        CHECK(decoder.is_valid());
        auto j = decoder.get_result();
        CHECK(j.as<std::string>() == s);
    }

}
#endif

