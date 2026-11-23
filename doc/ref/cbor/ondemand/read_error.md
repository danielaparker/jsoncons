### jsoncons::cbor::ondemand::read_error

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>

class read_error;
```

### class read_error

#### Accessors

    cbor_errc code() const noexcept;
Returns an error code.

    std::size_t position() const noexcept
Returns a position in the CBOR bytes

    std::string message() const;
Returns an error message


