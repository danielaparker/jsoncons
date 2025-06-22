### jsoncons::conv_error

```cpp
#include <jsoncons/conv_error.hpp>
```

<br>

`jsoncons::conv_error` defines an exception type for reporting type conversion failures.

![conv_error](./diagrams/conv_error.png)

std::exception

#### Constructors

    conv_error(std::error_code ec);

    conv_error(std::error_code ec, const std::string& what_arg);

    conv_error(std::error_code ec, const char* what_arg);

    conv_error(const conv_error& other);

    conv_error(conv_error&& other);

#### Member functions

    const char* what() const noexcept final
Returns an error message

#### Inherited from std::system_error

    const std::error_code code() const noexcept
Returns an error code for this exception

### Example

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

int main()
{
    try
    {
        jsoncons::json j(10);
        auto sv = j.as<std::string_view>();
    }
    catch (const jsoncons::conv_error& e)
    {
        std::cout << e.what() << "\n";
    }
}
```

Output:
```
Cannot convert to string
```
