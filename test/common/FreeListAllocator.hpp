// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#ifndef SAMPLE_ALLOCATORS_HPP
#define SAMPLE_ALLOCATORS_HPP

#include <algorithm>
#include <iostream>
#include <memory>
#include <list>
#include <type_traits>
#include <utility>

// From http://coliru.stacked-crooked.com/a/cfd0c5c5021596ad

template <typename T>
class FreeListAllocator {
    union node {
        node* next;
        alignas(alignof(T)) unsigned char storage[sizeof(T)];
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
    int id_;

public:
    using value_type = T;
    using size_type = std::size_t;
    using propagate_on_container_move_assignment = std::true_type;
    
    FreeListAllocator(int id) noexcept 
        : id_(id) {}
    FreeListAllocator(const FreeListAllocator& other) noexcept : id_(other.id_) {}
    template <typename U>
    FreeListAllocator(const FreeListAllocator<U>& other) noexcept : id_(other.id()) {}
    FreeListAllocator(FreeListAllocator&& other) noexcept : id_(other.id_), list(other.list) {
        other.id_ = -1;                                                       
        other.list = nullptr;
    }

    FreeListAllocator& operator = (const FreeListAllocator& other) noexcept {
        id_ = other.id_;
        return *this;
    }

    FreeListAllocator& operator = (FreeListAllocator&& other) noexcept {
        clear();
        id_ = other.id_;
        list = other.list;
        other.id_ = -1;
        other.list = nullptr;
        return *this;
    }

    ~FreeListAllocator() noexcept { clear(); }
    
    int id() const noexcept
    {
        return id_;
    }

    friend bool operator==(const FreeListAllocator& lhs, const FreeListAllocator& rhs) noexcept
    {
        return lhs.id_ == rhs.id_;
    }

    friend bool operator!=(const FreeListAllocator& lhs, const FreeListAllocator& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    T* allocate(size_type n) {
        //std::cout << "Allocate(" << n << ") from ";
        if (n == 1) {
            auto ptr = list;
            if (ptr) {
                //std::cout << "freelist\n";
                list = list->next;
            } else {
                //std::cout << "new node\n";
                ptr = new node;
            }
            return reinterpret_cast<T*>(ptr);
        }

        //std::cout << "::operator new\n";
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* ptr, size_type n) noexcept {
        //std::cout << "Deallocate(" << static_cast<void*>(ptr) << ", " << n << ") to ";
        if (n == 1) {
            //std::cout << "freelist\n";
            auto node_ptr = reinterpret_cast<node*>(ptr);
            node_ptr->next = list;
            list = node_ptr;
        } else {
            //std::cout << "::operator delete\n";
            ::operator delete(ptr);
        }
    }

    template <typename U>
    struct rebind
    {
        using other = FreeListAllocator<U>;
    };
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
};

#endif
