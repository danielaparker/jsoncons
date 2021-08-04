### jsoncons::json_merge_patch::apply_merge_patch

```c++
#include <jsoncons_ext/json_merge_patch/json_merge_patch.hpp>

template <class Json>
void apply_merge_patch(Json& target, const Json& patch); 
```

Applies a patch to a `json` document.

#### Return value

None

#### Exceptions

None expected.

### Examples

#### Apply a JSON Patch with two add operations

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/json_merge_patch/json_merge_patch.hpp>

using jsoncons::json;
namespace json_merge_patch = jsoncons::json_merge_patch;

int main()
{
    json doc = json::parse(R"(
{
         "title": "Goodbye!",
         "author" : {
       "givenName" : "John",
       "familyName" : "Doe"
         },
         "tags":[ "example", "sample" ],
         "content": "This will be unchanged"
}
    )");

    json doc2 = doc;

    json patch = json::parse(R"(
{
         "title": "Hello!",
         "phoneNumber": "+01-123-456-7890",
         "author": {
       "familyName": null
         },
         "tags": [ "example" ]
}
    )");

    json_merge_patch::apply_merge_patch(doc, patch);

    std::cout << "(1)\n" << pretty_print(doc) << std::endl;

    // Create a JSON Patch

    auto patch2 = json_merge_patch::from_diff(doc2,doc);

    std::cout << "(2)\n" << pretty_print(patch2) << std::endl;

    json_merge_patch::apply_merge_patch(doc2,patch2);

    std::cout << "(3)\n" << pretty_print(doc2) << std::endl;
}
```
Output:
```
(1)
{
    "author": {
        "familyName": null
    },
    "phoneNumber": "+01-123-456-7890",
    "tags": ["example"],
    "title": "Hello!"
}
(2)
{
    "author": {
        "givenName": "John"
    },
    "content": "This will be unchanged",
    "phoneNumber": "+01-123-456-7890",
    "tags": ["example"],
    "title": "Hello!"
}
```

