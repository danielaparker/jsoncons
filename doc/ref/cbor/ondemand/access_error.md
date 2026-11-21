### jsoncons::cbor::ondemand::access_error

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>

enum class access_errc;

class access_error;
```

### enum class access_errc

#### Member constants

Name                  | Definition 
----------------------|---------------
invalid_type          | Attempting to acces an item of the wrong type
indefinite            | Cannot access a view of an item with indefinite length
result_out_of_range   | A numeric item is out of range 

### class access_error

#### Accessors

    access_errc code() const noexcept;
Returns an error code.

    std::size_t position() const noexcept
Returns a position in the CBOR bytes

    std::string message() const;
Returns an error message


