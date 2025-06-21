### jsoncons::conversion_error

```cpp
#include <jsoncons/conversion_result.hpp>
```

class conversion_error;
```

#### Accessors

    std::error_code code() const noexcept;
Returns an error code.

    const std::string& message_arg() const noexcept
Returns an explanatory string

#### Non-member functions

std::string to_string(const read_error& err);

std::ostream& operator<<(std::ostream& os, const read_error& err);


