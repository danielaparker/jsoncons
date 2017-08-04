```c++
jsoncons::json::merge

void merge(const json& source);
```

Copies each element in source into *this. If there is an element in *this with key equivalent to the key of an element from source, 
then that element is not copied. 

### Parameters

    source
`json` object value

### Return value

None

### Exceptions

Throws `std::runtime_error` if source or *this are not json objects.

## Examples

### Merging `json`

```c++
json j = json::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

const json source = json::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

j1.merge(source);

std::cout << j << endl;
```
Output:

```json
{"a":1,"b":2,"c":3}
```

