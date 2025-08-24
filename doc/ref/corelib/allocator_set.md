### jsoncons::allocator_set

```cpp
#include <jsoncons/allocator_set.hpp>

template< 
    typename Alloc,typename TempAlloc
> allocator_set;
```

Member type                         |Definition
------------------------------------|------------------------------
`allocator_type`|`Alloc`
`temp_allocator_type`|`TempAlloc`

#### Constructors

    allocator_set(const Alloc& alloc=Alloc(), const TempAlloc& temp_alloc=TempAlloc())
Constructs an `allocator_set` with an allocator for result data and a
second allocator for temporary allocations. 

    allocator_set(const allocator_set& other)
Copy constructor. 

    allocator_set(allocator_set&& other)

Move constructor. 

#### Accessors

    Alloc get_allocator() const;
Returns an allocator object for result data

    TempAlloc get_temp_allocator() const;
Returns an allocator object for for temporary allocations

#### Helper functions

    template <typename Alloc,typename TempAlloc>
    allocator_set<Alloc,TempAlloc> make_alloc_set(                                    (since 1.4.0)
        const Alloc& alloc, const TempAlloc& temp_alloc);

Combines an allocator for result data and an allocator for temporary allocations into an `allocator_set` object,
deducing the allocator types from the types of the arguments.

    allocator_set<std::allocator<char>,std::allocator<char>> make_alloc_set()         (since 1.4.0)

Creates an `allocator_set<std::allocator<char>,std::allocator<char>>` object with default allocators for result data
and temporary allocations.

    template <typename Alloc>
    allocator_set<Alloc,std::allocator<char>> make_alloc_set(const Alloc& alloc)      (since 1.4.0)

Creates an `allocator_set` with the provided allocator for result data and
defaulting to a `std::allocator<char>` for temporary allocations. 

    template <typename TempAlloc>
    allocator_set<std::allocator<char>,TempAlloc> make_alloc_set(temp_alloc_arg_t,    (since 1.4.0)
        const TempAlloc& temp_alloc) 

Creates a `allocator_set` object, defaulting the result allocator type to `std::allocator<char>`
and deducing the temp allocator type from the type of the `temp_alloc` argument.

    template <typename Alloc,typename TempAlloc>
    allocator_set<Alloc,TempAlloc> combine_allocators(                                (deprecated since 1.4.0)
        const Alloc& alloc, const TempAlloc& temp_alloc);

Combines an allocator for result data and an allocator for temporary allocations into an `allocator_set` object,
deducing the allocator types from the types of the arguments.

    allocator_set<std::allocator<char>,std::allocator<char>> combine_allocators()     (deprecated since 1.4.0)

Creates an `allocator_set<std::allocator<char>,std::allocator<char>>` object with default allocators for result data
and temporary allocations.

    template <typename Alloc>
    allocator_set<Alloc,std::allocator<char>> combine_allocators(const Alloc& alloc)  (deprecated since 1.4.0)

Creates an `allocator_set` with the provided allocator for result data and
defaulting to a `std::allocator<char>` for temporary allocations. 

    template <typename TempAlloc>
    allocator_set<std::allocator<char>,TempAlloc> temp_allocator_only(                (deprecated since 1.4.0)
        const TempAlloc& temp_alloc) 

Creates a `allocator_set` object, defaulting the result allocator type to `std::allocator<char>`
and deducing the temp allocator type from the type of the `temp_alloc` argument.


