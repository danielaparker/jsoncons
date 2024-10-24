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
The allocator is used to allocate memory for a long string, byte string, array, or object,
and it is retained in the long string, byte string, array, or object itself.
For other data members the allocator argument is ignored. 

#### Move construction

`basic_json` move construction 

```
Json j1(std::move(j));
```

initializes `j1` with the pointer or non-pointer value in `j`, and changes the value in `j` to `null`. For example: 

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

If either `j` or `j1` hold pointer values, `basic_json` move assignment 

```
j1 = std::move(j);
```

swaps the two data member values. If both `j` and `j1` hold non-pointer values,
move assignment copies `j`'s value to `j1` and leaves `j` unchanged. For example:

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
