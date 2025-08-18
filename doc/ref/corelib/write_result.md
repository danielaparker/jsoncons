### jsoncons::write_result

```cpp
#include <jsoncons/ser_util.hpp>

using write_result = jsoncons::expected<void,std::error_code>;
```

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`value_type`|void
`error_type`|[std::error_code](https://en.cppreference.com/w/cpp/error/error_code.html)

#### Accessors  

    constexpr operator bool() const noexcept;  
    constexpr bool has_value() const noexcept;  
Checks whether the result contains an unexpected value

    constexpr read_error& error() & noexcept;  
    constexpr const read_error& error() const & noexcept;  
    constexpr read_error&& error() && noexcept;  
    constexpr const read_error&& error() const && noexcept;  
Returns the unexpected value [std::error_code](https://en.cppreference.com/w/cpp/error/error_code.html)



