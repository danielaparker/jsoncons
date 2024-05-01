### jsoncons::jsonschema::json_schema::is_valid

```cpp
bool is_valid(const Json& instance) const;
```

Validates input JSON against a JSON Schema and returns false upon the 
first schema violation.

#### Parameters

<table>
  <tr>
    <td>instance</td>
    <td>Input Json</td> 
  </tr>
</table>

#### Return value
 
`true` if the instance is valid, otherwise `false`.

#### Exceptions

None.

