### jsoncons::jmespath::jmespath_expression

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <class Json>
class jmespath_expression
```

#### Member functions

    Json evaluate(reference doc, std::error_code& ec); // (1)

    Json evaluate(reference doc, std::error_code& ec); // (2)

#### Exceptions

(1) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath compilation fails. 

(2) Throws a [jmespath_error](jmespath_error.md) if JMESPath evaluation fails.

#### Static functions

    static jmespath_expression compile(const string_view_type& expr); // (1)

    static jmespath_expression compile(const string_view_type& expr,
                                       std::error_code& ec); //(2)

Compiles the JMESPath expression and returns a `jmespath_expression` object 
that represents the JMESPath expression.

#### Exceptions

(1) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath compilation fails. 

(2) Throws a [jmespath_error](jmespath_error.md) if JMESPath compilation fails.

