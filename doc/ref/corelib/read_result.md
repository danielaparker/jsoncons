### jsoncons::read_result

```cpp
#include <jsoncons/read_result.hpp>
```

template <
    typename T
> class read_result
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

    constexpr read_error& error() & noexcept;

    constexpr const read_error& error() const & noexcept;

    constexpr read_error&& error() && noexcept;

    constexpr const read_error&& error() const && noexcept;


