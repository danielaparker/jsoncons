### jsoncons::wrapped_allocators

```cpp
#include <jsoncons/wrapped_allocators.hpp>

template< 
    class ResultAllocator,class TempAllocator
> wrapped_allocators;
```

Member type                         |Definition
------------------------------------|------------------------------
`result_allocator_type`|`ResultAllocator`
`temp_allocator_type`|`TempAllocator`

#### Constructors

    wrapped_allocators(const ResultAllocator& result_alloc, const TempAllocator& temp_alloc)
Constructs a `wrapped_allocators` with a result and a work allocator. 

    wrapped_allocators(const wrapped_allocators&)
Copy constructor. 

    wrapped_allocators(wrapped_allocators&&)
Move constructor. 

#### Accessors

    ResultAllocator get_result_allocator() const;

    TempAllocator get_temp_allocator() const;

#### Non member functions

    template <class ResultAllocator,class TempAllocator>
    wrapped_allocators<ResultAllocator,TempAllocator> wrap_allocators(
        const ResultAllocator& result_alloc = std::allocator<char>(), 
        const TempAllocator& temp_alloc = std::allocator<char>());

    Creates a `wrapped_allocators` object, deducing the allocator types from the types of arguments.

    template <class TempAllocator>
    wrapped_allocators<std::allocator<char>,TempAllocator> wrap_allocators(temp_allocator_arg_t, 
        const TempAllocator& temp_alloc)

    Creates a `wrapped_allocators` object, defaulting the result allocator type to `std::allocator<char>`
    and deducing the work allocator type from the type of the `temp_alloc` argument.


