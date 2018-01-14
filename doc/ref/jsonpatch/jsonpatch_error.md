### jsoncons::jsonpointer::jsonpatch_error

#### Header

    #include <jsoncons/jsonpointer/jsonpointer.hpp>

### Base class

std::exception

#### Constructors

    jsonpatch_error(std::error_code ec)

    jsonpatch_error(const jsonpatch_error& other)

#### Member functions

    const std::error_code code() const
Returns an error code for this exception

    const char* what() const
Returns an error message

### Example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using jsoncons::json;

int main()
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] },
            { "op": "add", "path": "/baz/bat", "value": "qux" } // nonexistent target
        ]
    )"_json;

    try
    {
        jsonpatch::apply_patch(target, patch);
    }
    catch (const jsonpatch::jsonpatch_error& e)
    {
        std::cout << "(1) " << e.what() << std::endl;
        std::cout << "(2) " << target << std::endl;
    }
}
```

Output:
```
(1) JSON Patch add operation failed
(2) {"foo":"bar"}
```
