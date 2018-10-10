### jsoncons::encode_json

Serializes a C++ object to a JSON formatted string or stream. `encode_json` will work for all types that
have [json_type_traits](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_type_traits.md) defined.

#### Header
```c++
#include <jsoncons/json.hpp>

template <class T, class CharT>
void encode_json(const T& val, basic_json_content_handler<CharT>& handler); // (1)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os); // (2)

template <class T, class CharT>
void encode_json(const T& val, 
                 const basic_json_serializing_options<CharT>& options,
                 std::basic_ostream<CharT>& os); // (3)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, indenting line_indent); // (4)

template <class T, class CharT>
void encode_json(const T& val, 
                 const basic_json_serializing_options<CharT>& options,
                 std::basic_ostream<CharT>& os, indenting line_indent); // (5)
```

(1) Applies `json_convert_traits` to serialize `val` to JSON output stream.

#### Parameters

<table>
  <tr>
    <td>val</td>
    <td>C++ object</td> 
  </tr>
  <tr>
    <td>handler</td>
    <td>JSON output handler</td> 
  </tr>
  <tr>
    <td>options</td>
    <td>Serialization options</td> 
  </tr>
  <tr>
    <td>os</td>
    <td>Output stream</td> 
  </tr>
  <tr>
    <td>indenting</td>
    <td><code>indenting::indent</code> to pretty print, <code>indenting::no_indent</code> for compact output</td> 
  </tr>
</table>

#### Return value

None 

#### See also

- [json_content_handler](json_content_handler.md)
- [json_serializing_options](json_serializing_options.md)
    
### Examples

#### Map with string-tuple pairs

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    encode_json(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    encode_json(employees, std::cout, jsoncons::indenting::indent);
}
```
Output:
```
(1)

{"Jane Doe":["Commission","Sales",20000.0],"John Smith":["Hourly","Software Engineer",10000.0]}

(2) Again, with pretty print

{
    "Jane Doe": ["Commission","Sales",20000.0],
    "John Smith": ["Hourly","Software Engineer",10000.0]
}
```
    
#### Contain JSON output in an object

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    json_serializer serializer(std::cout, jsoncons::indenting::indent); 

    serializer.begin_object();       
    serializer.write_name("Employees");       
    encode_json(employees, serializer);
    serializer.end_object();       
    serializer.flush();       
}
```
Output:
```json
{
    "Employees": {
        "Jane Doe": ["Commission","Sales",20000.0],
        "John Smith": ["Hourly","Software Engineer",10000.0]
    }
}
```

#### See also

- [decode_json](decode_json.md)


