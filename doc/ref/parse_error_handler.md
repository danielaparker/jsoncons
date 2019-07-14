### jsoncons::parse_error_handler

```c++
#include <jsoncons/json_parser.hpp>
```

A `parse_error_handler` is a simple function, a functor or a lambda expression that receive arguments 
`std::error_code` and const `ser_context&`, and returns a `bool`. The parser will report all errors
through the `parse_error_handler`. If the handler `true`, the parser
will make an attempt to recover from recoverable errors, if the error is non-recoverable of if the handler
returns `false`, the parser will stop. 

The jsoncons library comes with two `parse_error_handler` handlers:

- `default_json_parsing`, which returns `true` if the error code indicates a comment, otherwise `false`

- `strict_json_parsing`, which always returns `false`



    

