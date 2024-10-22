### Allocators


A `basic_json` is a union type that can hold one of a number of possible data members, 
some of which require an allocator (a long string, byte string, array, or object), 
and others that do not (a scalar, i.e., an empty object, short string, number, boolean, or null). 
It has an `Allocator` template parameter and an `allocator_type` member that indicates
that it is allocator aware. 

Every constructor has a version that can be invoked with an allocator argument. 
This argument is used to allocate memory for a long string, byte string, array, or object. 
In the case of scalars, it is ignored.

Scoped Allocators are allocators that apply not only to the top-level container, but also to its elements 

#### https://en.cppreference.com/w/cpp/memory/polymorphic_allocator


#### std::scoped_allocator_adaptor

Non-propagating stateful allocators, such as the [Boost.Interprocess allocators](https://www.boost.org/doc/libs/1_82_0/doc/html/interprocess/allocators_containers.html#interprocess.allocators_containers.allocator_introduction),
must be wrapped by a [std::scoped_allocator_adaptor](https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor).

