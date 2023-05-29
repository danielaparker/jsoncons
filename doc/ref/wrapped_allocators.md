### jsoncons::wrapped_allocators

```cpp
#include <jsoncons/wrapped_allocators.hpp>

template< 
    class ResultAllocator,class WorkAllocator
> wrapped_allocators;
```

Member type                         |Definition
------------------------------------|------------------------------
`result_allocator_type`|`ResultAllocator`
`work_allocator_type`|`WorkAllocator`

#### Constructors

    wrapped_allocators(const ResultAllocator& result_alloc, const WorkAllocator& work_alloc)
Constructs a `wrapped_allocators` with a result and a work allocator. 

    wrapped_allocators(const wrapped_allocators&)
Copy constructor. 

    wrapped_allocators(wrapped_allocators&&)
Move constructor. 

#### Accessors

    ResultAllocator get_result_allocator() const;

    WorkAllocator get_work_allocator() const;

#### Non member functions

    template <class ResultAllocator,class WorkAllocator>
    wrapped_allocators<ResultAllocator,WorkAllocator> wrap_allocators(
        const ResultAllocator& result_alloc = std::allocator<char>(), 
        const WorkAllocator& work_alloc = std::allocator<char>());

    Creates a `wrapped_allocators` object, deducing the allocator types from the types of arguments.

    template <class WorkAllocator>
    wrapped_allocators<std::allocator<char>,WorkAllocator> wrap_allocators(work_allocator_arg_t, 
        const WorkAllocator& work_alloc)

    Creates a `wrapped_allocators` object, defaulting the result allocator type to `std::allocator<char>`
    and deducing the work allocator type from the type of the `work_allocator` argument.

#### Helper classes

    struct work_allocator_arg_t {explicit work_allocator_arg_t() = default;};

An empty class type used to disambiguate the constructors of `wrapped_allocators`. 
`work_allocator_arg` is a constant of type `work_allocator_arg_t`.
