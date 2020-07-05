### jsoncons::jmespath::jmespath_expression

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <class Json>
class jmespath_expression
```

#### Member functions

#### Static functions

    static jmespath_expression compile(const string_view_type& expr); // (1)

    static jmespath_expression compile(const string_view_type& expr,
                                       std::error_code& ec); //(2)

### Examples

