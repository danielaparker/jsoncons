### jsoncons::jsonpatch::patch

Patch a `json` document.

#### Header
```c++
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

template <class Json>
std::tuple<jsonpatch_errc,typename Json::string_type> patch(Json& target, const Json& patch)
```

#### Return value

On success, returns a value-initialized [jsonpatch_errc](jsonpatch_errc.md) and an empty string value.  

On error, returns a [jsonpatch_errc](jsonpatch_errc.md) error code and the path that failed. 

### Examples

#### Apply a JSON Patch with two add operations

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

int main()
{
    json target = R"(
        { "foo": "bar"}
    )"_json;

    json patch = R"(
        [
            { "op": "add", "path": "/baz", "value": "qux" },
            { "op": "add", "path": "/foo", "value": [ "bar", "baz" ] }
        ]
    )"_json;

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(target,patch);

    std::cout << pretty_print(target) << std::endl;
}
```
Output:
```
{
    "baz": "qux",
    "foo": ["bar","baz"]
}
```

#### Apply a JSON Patch with three add operations, the last one fails

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

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

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(target,patch);

    std::cout << "(1) " << std::error_code(ec).message() << std::endl;
    std::cout << "(2) " << path << std::endl;
    std::cout << "(3) " << target << std::endl;
}
```
Output:
```
(1) JSON Patch add operation failed
(2) /baz/bat
(3) {"foo":"bar"}
```
Note that all JSON Patch operations have been rolled back, and target is in its original state.

