// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <vector>
#include <memory>
#include <iostream>

#include <catch/catch.hpp>

template <typename T>
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

    mock_stateful_allocator() = delete;

    mock_stateful_allocator(int id) noexcept
        : impl_(), id_(id)
    {
    }

    mock_stateful_allocator(const mock_stateful_allocator<T>& other) noexcept = default;

    template <typename U>
    friend class mock_stateful_allocator;

    template <typename U>
    mock_stateful_allocator(const mock_stateful_allocator<U>& other) noexcept
        : impl_(), id_(other.id_)
    {
    }

    mock_stateful_allocator& operator = (const mock_stateful_allocator& other) = delete;

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

template <typename Alloc>
struct allocator_delete : public Alloc
{
    using allocator_type = Alloc;
    using alloc_traits = std::allocator_traits<Alloc>;
    using pointer = typename std::allocator_traits<Alloc>::pointer;
    using value_type = typename std::allocator_traits<Alloc>::value_type;

    allocator_delete(const Alloc& alloc) noexcept
        : Alloc(alloc) {
    }

    allocator_delete(const allocator_delete&) noexcept = default;

    template <typename T>
    typename std::enable_if<std::is_convertible<T&, value_type&>::value>::type
        operator()(T* ptr)
    {
        std::cout << "type: " << typeid(*ptr).name() << "\n";
        alloc_traits::destroy(*this, ptr);
        alloc_traits::deallocate(*this, ptr, 1);
    }
};

struct Foo
{
    virtual ~Foo()
    {
    }
};

struct Bar : public Foo
{
    int x = 0;
};

TEST_CASE("allocator_delete tests")
{
    using allocator_type = mock_stateful_allocator<Foo>;
    using deleter_type = allocator_delete<allocator_type>;
    using value_type = std::unique_ptr<Foo,deleter_type>;
    
    std::vector<value_type> v{};

    using rebind = typename std::allocator_traits<allocator_type>::template rebind_alloc<Bar>; 
    rebind alloc(1);
    auto* p = alloc.allocate(1);
    p = new(p)Bar();

    v.push_back(value_type(p, deleter_type(alloc)));

    std::cout << "sizeof(Foo): " << sizeof(Foo) << ", sizeof(Bar): " << sizeof(Bar) << "\n";
}

