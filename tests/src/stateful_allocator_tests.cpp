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

    template<typename U>
    struct rebind
    {
        typedef FreelistAllocator<U> other;
    };
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::ptrdiff_t difference_type;
};

template <typename T, typename U>
inline bool operator == (const FreelistAllocator<T>&, const FreelistAllocator<U>&) {
    return true;
}

template <typename T, typename U>
inline bool operator != (const FreelistAllocator<T>&, const FreelistAllocator<U>&) {
    return false;
}

#if !(defined(__GNUC__) && __GNUC__ == 4)
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

