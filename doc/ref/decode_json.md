### jsoncons::decode_json

Converts a JSON formatted string to a C++ object. `decode_json` attempts to 
perform the conversion by streaming using `json_convert_traits`, and if
streaming is not supported, falls back to using `json_type_traits`. `decode_json` will 
work for all C++ classes that have `json_type_traits` defined.

#### Header

```c++
#include <jsoncons/json.hpp>

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s);  // (1)

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s,
              const basic_json_serializing_options<CharT>& options); // (2)

template <class T, class CharT>
T decode_json(std::basic_istringstream<CharT>& is); // (3)

template <class T, class CharT>
T decode_json(std::basic_istringstream<CharT>& is,
              const basic_json_serializing_options<CharT>& options); // (4)
```

### Examples

#### Map with string-tuple pairs

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

int main()
{
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    std::string s = R"(
    {
        "Jane Doe": ["Commission","Sales",20000.0],
        "John Smith": ["Hourly","Software Engineer",10000.0]
    }
    )";

    employee_collection employees = jsoncons::decode_json<employee_collection>(s);

    for (const auto& pair : employees)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
```
Output:
```
Jane Doe: Sales
John Smith: Software Engineer
```

#### See also

- [encode_json](encode_json.md)

