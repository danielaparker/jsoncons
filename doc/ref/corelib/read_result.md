### jsoncons::read_result

```cpp
#include <jsoncons/ser_util.hpp>

template <
    typename T
> class read_result
```

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`value_type`|`T`
`error_type`|[read_error](read_error.md)

#### Accessors  

    constexpr const T* operator->() const noexcept;  
    constexpr T* operator->() noexcept;  
    constexpr const T& operator*() const & noexcept;  
    constexpr T& operator*() & noexcept;  
    constexpr const T&& operator*() const && noexcept;  
    constexpr T&& operator*() && noexcept;  
Accesses the expected value

    constexpr operator bool() const noexcept;  
    constexpr bool has_value() const noexcept;  
Checks whether the result contains an expected value

    constexpr T& value() &;  
    constexpr const T& value() const &;  
    constexpr T&& value() &&;  
    constexpr const T&& value() const &&;  
Returns the expected value

    constexpr read_error& error() & noexcept;  
    constexpr const read_error& error() const & noexcept;  
    constexpr read_error&& error() && noexcept;  
    constexpr const read_error&& error() const && noexcept;  
Returns the unexpected value [read_error](read_error.md)



