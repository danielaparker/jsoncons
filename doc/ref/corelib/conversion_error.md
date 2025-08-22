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

    std::string message() const;
Returns an error message


