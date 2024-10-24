### Allocators

A `basic_json` has an `Allocator` template parameter and an `allocator_type` member that indicates
that it is allocator aware. `Allocator` must be a Scoped Allocator, that is, an allocator 
that applies not only to a `basic_json`'s data member, but also to its data member's elements.
In particular, `Allocator` must be either a stateless allocator, 
a <a href=https://en.cppreference.com/w/cpp/memory/polymorphic_allocator>std::pmr::polymorphic_allocator</a>, 
or a <a href=https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor>std::scoped_allocator_adaptor</a>. 
Non-propagating stateful allocators, such as the [Boost.Interprocess allocators](https://www.boost.org/doc/libs/1_82_0/doc/html/interprocess/allocators_containers.html#interprocess.allocators_containers.allocator_introduction),
must be wrapped by a [std::scoped_allocator_adaptor](https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor).

Every constructor has a version that accepts an allocator argument. 
A long string, byte string, array or object contains a pointer to underlying storage,
the allocator is used to allocate that storage, and it is retained in that storage.
For other data members the allocator argument is ignored.

#### Copy construction

`basic_json` copy construction 

```
Json j1(j);
```

constructs `j1` from `j`. If `j` is a long string, bytes string, array or object,
copy construction uses allocator traits `select_on_container_copy_construction` to obtain
an allocator (since 0.178.0) For example: 

```cpp
char buffer[1024];
std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
std::pmr::polymorphic_allocator<char> alloc(&pool);

jsoncons::pmr::json j{ "String too long for short string", alloc };

jsoncons::pmr::json j1(j);
assert(j1.is_string());
assert(j1.get_allocator() == std::allocator_traits<std::pmr::polymorphic_allocator<char>>::
    select_on_container_copy_construction(j.get_allocator()));
assert(j1.get_allocator() == std::pmr::polymorphic_allocator<char>{}); // expected result for pmr allocators
```

#### Move construction

`basic_json` move construction 

```
Json j1(std::move(j));
```

initializes `j1` with the contents of `j`, which is either a pointer or a trivially copyable value,
and changes the value in `j` to `null`. For example: 

```
char buffer[1024] = {};
std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
std::pmr::polymorphic_allocator<char> alloc(&pool);

jsoncons::pmr::json j{ "String too long for short string", alloc};
assert(j.is_string());
assert(alloc == j.get_allocator());

jsoncons::pmr::json j1(std::move(j));
assert(j1.is_string());
assert(alloc == j1.get_allocator());
assert(j.is_null());
```

#### Move assignment

If either `j` or `j1` are a long string, byte string, array, or object, `basic_json` move assignment 

```
j1 = std::move(j);
```

swaps the two data member values, such that two pointers are swapped or a pointer and a
trivially copyable value are swapped. Otherwise, move assignment copies `j`'s value to `j1`
and leaves `j` unchanged. For example:

```cpp
char buffer[1024];
std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
std::pmr::polymorphic_allocator<char> alloc(&pool);

jsoncons::pmr::json j{ "String too long for short string", alloc };
assert(j.is_string());
assert(alloc == j.get_allocator());

jsoncons::pmr::json j1{10};
assert(j1.is_number());

j1 = std::move(j);
assert(j1.is_string());
assert(alloc == j1.get_allocator());
assert(j.is_number());
```
