### jsoncons::conversion_result

```cpp
#include <jsoncons/conversion_result.hpp>


template <typename T> 
using conversion_result = jsoncons::expected<T,conversion_error>;
```

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`value_type`|`T`
`error_type`|[conversion_error](conversion_error.md)

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

    constexpr conversion_error& error() & noexcept;  
    constexpr const conversion_error& error() const & noexcept;  
    constexpr conversion_error&& error() && noexcept;  
    constexpr const conversion_error&& error() const && noexcept;  
Returns the unexpected value [conversion_error](conversion_error.md)


