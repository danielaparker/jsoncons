### jsoncons::read_error

```cpp
#include <jsoncons/read_result.hpp>
```

class read_error;
```

#### Accessors

    std::error_code code() const noexcept;
Returns an error code.

    const std::string& message_arg() const noexcept
Returns an explanatory string

    std::size_t line() const noexcept
Returns the line number to the end of the text where the exception occurred.
Line numbers start at 1.

    std::size_t column() const noexcept
Returns the column number to the end of the text where the exception occurred.
Column numbers start at 1.

#### Non-member functions

std::string to_string(const read_error& err);

std::ostream& operator<<(std::ostream& os, const read_error& err);


