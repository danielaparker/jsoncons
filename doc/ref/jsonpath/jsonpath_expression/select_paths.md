### jsoncons::jsonpath::jsonpath_expression::select

```cpp
std::vector<basic_json_location<char_type>> select_paths(const_reference root_value,
    result_options options = result_options::nodups | result_options::sort);                                                 (1) (since 0.172.0)
```

(1) Evaluates the root value against the compiled JSONPath expression and returns the
locations of the selected values in the root value.

#### Parameters

<table>
  <tr>
    <td>root_value</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td><code>options</code> (since 0.161.0)</td>
    <td>Result options, a bitmask of type <a href="../result_options.md">result_options</></td> 
  </tr>
</table>

