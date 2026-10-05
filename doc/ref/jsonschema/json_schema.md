### jsoncons::jsonschema::json_schema

```cpp
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

template <typename Json>
class json_schema
```

A `json_schema` represents the compiled form of a JSON Schema document.
A `json_schema` is immutable and thread-safe.

The class satisfies the requirements of MoveConstructible and MoveAssignable, but not CopyConstructible or CopyAssignable.

`Json` must be a `basic_json` specialization with `char_type` equal to `char`
and an allocator that is either polymorphic or always equal
(`std::allocator_traits<Json::allocator_type>::is_always_equal::value`).
This includes `json`, `ojson`, and their `jsoncons::pmr` variants. Wide-character
types such as `wjson` and `wojson`, and other stateful allocators, are not supported.
Instantiating `json_schema` with these unsupported types produces a compile-time diagnostic.

#### Member functions

<table border="0">
  <tr>
    <td><a href="json_schema/is_valid.md">is_valid</a></td>
    <td>Validates input JSON against a JSON Schema and returns false upon the first schema violation</td> 
  </tr>
  <tr>
    <td><a href="json_schema/validate.md">validate</a></td>
    <td>Validates input JSON against a JSON Schema.</td> 
  </tr>
  <tr>
    <td><a href="json_schema/walk.md">walk</a> (since 0.175.0)</td>
    <td>Walks through a JSON Schema.</td> 
  </tr>
</table>
