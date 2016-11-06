// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(allocator_test_suite)

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
    typedef std::aligned_storage<sizeof(node_typeA), JSONCONS_ALIGNOF(node_typeA)>::type storage_type;

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
        : offset_(0), size_(size) 
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
    template<typename U> 
    struct rebind 
    {
        typedef pool_allocator<U> other;
    }; 
    pool_allocator(pool* pp) throw() : pool_ptr_(pp) 
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
    ~pool_allocator() throw() 
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
        return static_cast<T*>(pool_ptr_->allocate(n * sizeof(T)));
    } 
    void deallocate(pointer, size_type) 
    {
    }
    size_type max_size() const throw() 
    {
        return size_t(-1) / sizeof(T);
    } 
    void construct(pointer p, const T& val) 
    {
        ::new(p) T(val);
    } 
    void destroy(pointer p) 
    {
        p->~T();
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

BOOST_AUTO_TEST_CASE(test_allocator)
{
    pool a_pool(1024);
    pool_allocator<json> a_pool_allocator(&a_pool); 

    typedef basic_json<char,json_traits<char>,pool_allocator<json>> myjson;
    myjson::array an_array = myjson::array(a_pool_allocator);
    myjson::object an_object = myjson::object(a_pool_allocator);

    myjson root = an_object;
    root.reserve(1);

    /*root.set("field1", 10.0);
    root.set("field2", 20.0);

    auto it = root.find("field1");

    std::cout << it->key() << std::endl;
    if (it != root.object_range().end())
    {
        myjson& val = it->value();
    }*/
   
    //BOOST_CHECK_EQUAL(10.0,root["field1"]);
    //BOOST_CHECK_EQUAL(20.0,root.get("field1",0));
    //std::cout << root << std::endl; 
}
BOOST_AUTO_TEST_SUITE_END()
