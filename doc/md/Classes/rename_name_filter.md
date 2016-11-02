```
jsoncons::rename_name_filter

typedef basic_rename_name_filter<char> rename_name_filter;
```
The `rename_name_filter` class is an instantiation of the `basic_rename_name_filter` class template that uses `char` as the character type. 

Renames object member names. 

### Header
```c++
#include <jsoncons/json_filter.hpp>
```

### Base classes

[json_filter](json_filter)

### Constructors

    rename_name_filter(const std::string& name,
                       const std::string& new_name,
                       json_output_handler& handler)

    rename_name_filter(const std::string& name,
                       const std::string& new_name,
                       json_input_handler& handler)

## Examples

### Rename object member names

```c++
#include "jsoncons/json.hpp"
#include "jsoncons/json_filter.hpp"

using namespace jsoncons;

int main()
{
    ojson j = ojson::parse(R"({"first":1,"second":2,"fourth":3})");

    json_serializer serializer(std::cout);

    rename_name_filter filter("fourth","third",serializer);
    j.write(filter);
}
```
Output:
```json
{"first":1,"second":2,"third":3}
```

