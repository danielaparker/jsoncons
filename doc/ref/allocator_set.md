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

    allocator_set(const Allocator& alloc=Allocator(), 
        const TempAllocator& temp_alloc=TempAllocator())
Constructs a `allocator_set` with a result and a work allocator. 

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

    template <class Allocator,class TempAllocator>
    allocator_set<Allocator,TempAllocator> make_allocator_set(
        const Allocator& alloc = std::allocator<char>(), 
        const TempAllocator& temp_alloc = std::allocator<char>());

    Creates a `allocator_set` object, deducing the allocator types from the types of arguments.

    template <class TempAllocator>
    allocator_set<std::allocator<char>,TempAllocator> make_allocator_set(temp_allocator_arg_t, 
        const TempAllocator& temp_alloc)

    Creates a `allocator_set` object, defaulting the result allocator type to `std::allocator<char>`
    and deducing the work allocator type from the type of the `temp_alloc` argument.


