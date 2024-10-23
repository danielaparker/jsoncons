### Allocators


A `basic_json` has an `Allocator` template parameter and an `allocator_type` member that indicates
that it is allocator aware. `Allocator` must be a Scoped Allocator, that is, an allocator 
that applies not only to a `basic_json`'s data member, but also to its data member's elements.
In particular, `Allocator` must be either a stateless allocator, 
a <a href=https://en.cppreference.com/w/cpp/memory/polymorphic_allocator>std::pmr::polymorphic_allocator</a>, 
or a <a href=https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor>std::scoped_allocator_adaptor</a>. 
Non-propagating stateful allocators, such as the [Boost.Interprocess allocators](https://www.boost.org/doc/libs/1_82_0/doc/html/interprocess/allocators_containers.html#interprocess.allocators_containers.allocator_introduction),
must be wrapped by a [std::scoped_allocator_adaptor](https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor).

Every constructor has a version that can be invoked with an allocator argument. 
This argument is used to allocate memory for a long string, byte string, array, or object. 
In the case of scalars, it is ignored. 

