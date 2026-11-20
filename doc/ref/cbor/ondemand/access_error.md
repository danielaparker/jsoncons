### jsoncons::cbor::ondemand::access_error

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

enum class access_errc : uint8_t 
{
    success = 0,
    invalid_type = 1,
    indefinite,
    result_out_of_range, 
};

class access_error;
```

#### Accessors

    access_errc code() const noexcept;
Returns an error code.

    std::size_t position() const noexcept
Returns a position in the CBOR bytes

    std::string message() const;
Returns an error message


