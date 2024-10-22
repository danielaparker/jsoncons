### Allocators

A [basic_json](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/corelib/basic_json.md) is a union type 
that can hold one of a number of possible data members, some of which require an allocator (a long string, byte string, array, or object), 
and others which do not (an empty object, short string, number, boolean, or null). 

#### Polymorphic Allocators


#### Scoped Allocators

Non-propagating stateful allocators, such as the [Boost.Interprocess allocators](https://www.boost.org/doc/libs/1_82_0/doc/html/interprocess/allocators_containers.html#interprocess.allocators_containers.allocator_introduction),
must be wrapped by a [std::scoped_allocator_adaptor](https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor).

