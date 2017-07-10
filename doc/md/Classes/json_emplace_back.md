```
jsoncons::json::emplace_back

template<class... Args>
json_type& emplace_back(Args&&... args);
```

### Parameters

    args 
Arguments to forward to the constructor of the json value

### Return value

A reference to the emplaced json value.

### Exceptions

Throws `std::runtime_error` if not a json array.

### Example

```c++
json a = json::array();
a.emplace_back("Toronto");
a.emplace_back("Vancouver");
a.emplace(a.array_range().begin(),"Montreal");

std::cout << a << std::endl;
```
Output:

```json
["Montreal","Toronto","Vancouver"]
```

