### jsoncons::json_merge_patch::from_diff

```c++
#include <jsoncons_ext/json_merge_patch/json_merge_patch.hpp>

template <class Json>
Json from_diff(const Json& source, const Json& target)
```

Create a JSON Merge Patch from a diff of two json documents.

#### Return value

Returns a JSON Merge Patch.  

### Examples

#### Create a JSON Merge Patch

This example is from [RFC 7386](https://datatracker.ietf.org/doc/html/rfc7386#section-3).

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/json_merge_patch/json_merge_patch.hpp>

using jsoncons::json;
namespace json_merge_patch = jsoncons::json_merge_patch;

int main()
{
    json source = json::parse(R"(
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

    json target = json::parse(R"(
{
  "title": "Hello!",
  "author": {
    "givenName": "John"
  },
  "tags": [
    "example"
  ],
  "content": "This will be unchanged",
  "phoneNumber": "\u002B01-123-456-7890"
}
    )");

    auto patch = json_merge_patch::from_diff(source, target);

    json_merge_patch::apply_merge_patch(source, patch);

    std::cout << "(1)\n" << pretty_print(patch) << std::endl;
    std::cout << "(2)\n" << pretty_print(source) << std::endl;
}
```
Output:
```
(1)
{
    "author": {
        "givenName": "John"
    },
    "content": "This will be unchanged",
    "phoneNumber": "+01-123-456-7890",
    "tags": ["example"],
    "title": "Hello!"
}
(2)
{
    "author": {
        "familyName": null
    },
    "phoneNumber": "+01-123-456-7890",
    "tags": ["example"],
    "title": "Hello!"
}
(3)
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

