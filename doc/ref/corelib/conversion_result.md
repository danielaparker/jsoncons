### jsoncons::conversion_result

```cpp
#include <jsoncons/conversion_result.hpp>
```

template <
    typename T
> class conversion_result
```

#### Accessors

    constexpr const T* operator->() const noexcept;

    constexpr T* operator->() noexcept;

    constexpr const T& operator*() const & noexcept;

    constexpr T& operator*() & noexcept;

    constexpr const T&& operator*() const && noexcept;

    constexpr T&& operator*() && noexcept;

    constexpr operator bool() const noexcept;
    
    constexpr bool has_value() const noexcept;

    constexpr T& value() &;

    constexpr const T& value() const &;

    constexpr T&& value() &&;

    constexpr const T&& value() const &&;

    constexpr conversion_error& error() & noexcept;

    constexpr const conversion_error& error() const & noexcept;

    constexpr conversion_error&& error() && noexcept;

    constexpr const conversion_error&& error() const && noexcept;


