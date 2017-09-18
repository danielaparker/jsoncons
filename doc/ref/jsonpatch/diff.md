### jsoncons::jsonpatch::diff

Create a diff as a JSON Patch.

#### Header
```c++
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

template <class Json>
Json diff(const Json& source, const Json& target)
```

#### Return value

Returns a JSON Patch.  

### Examples

#### Create a JSON Patch

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

int main()
{
    json source = R"(
        {"/": 9, "foo": "bar"}
    )"_json;

    json target = R"(
        { "baz":"qux", "foo": [ "bar", "baz" ]}
    )"_json;

    auto patch = jsonpatch::diff(source, target);

    jsonpatch::jsonpatch_errc ec;
    std::string path;
    std::tie(ec,path) = jsonpatch::patch(source,patch);

    std::cout << "(1) " << pretty_print(patch) << std::endl;
    std::cout << "(2) " << pretty_print(source) << std::endl;
}
```
Output:
```
(1) 
[
    {
        "op": "remove",
        "path": "/~1"
    },
    {
        "op": "replace",
        "path": "/foo",
        "value": ["bar","baz"]
    },
    {
        "op": "add",
        "path": "/baz",
        "value": "qux"
    }
]
(2) 
{
    "baz": "qux",
    "foo": ["bar","baz"]
}
```

