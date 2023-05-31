### jsoncons::allocator_set

```cpp
#include <jsoncons/allocator_set.hpp>

template< 
    class Allocator,class TempAllocator
> allocator_set;
```

Member type                         |Definition
------------------------------------|------------------------------
`allocator_type`|`Allocator`
`temp_allocator_type`|`TempAllocator`

#### Constructors

    allocator_set(const Allocator& alloc=Allocator(), const TempAllocator& temp_alloc)
Constructs an `allocator_set` with an allocator for persistent data and a
second allocator for temporary allocations. 

    allocator_set(const allocator_set&)
Copy constructor. 

    allocator_set(allocator_set&&)

Move constructor. 

#### Accessors

    Allocator get_allocator() const;
An allocator object for persistent data

    TempAllocator get_temp_allocator() const;
An allocator object for for temporary allocations

#### Non member functions

    allocator_set<std::allocator<char>,std::allocator<char>> make_allocator_set()
Creates a `allocator_set<std::allocator<char>,std::allocator<char>>` object.

    template <class Allocator>
    allocator_set<Allocator,std::allocator<char>> make_allocator_set(
        const Allocator& alloc);

Creates an `allocator_set` with the provided allocator for persistent data and
defaulting to a `std::allocator<char>` for temporary allocations. 

    template <class Allocator,class TempAllocator>
    allocator_set<Allocator,TempAllocator> make_allocator_set(
        const Allocator& alloc = std::allocator<char>(), 
        const TempAllocator& temp_alloc = std::allocator<char>());

Creates an `allocator_set` object, deducing the allocator types from the types of the arguments.

    template <class TempAllocator>
    allocator_set<std::allocator<char>,TempAllocator> make_allocator_set(temp_allocator_arg_t, 
        const TempAllocator& temp_alloc)

Creates a `allocator_set` object, defaulting the result allocator type to `std::allocator<char>`
and deducing the temp allocator type from the type of the `temp_alloc` argument.


