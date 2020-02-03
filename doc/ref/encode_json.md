### jsoncons::encode_json

__`jsoncons/json.hpp`__

Encode a C++ data structure to a JSON formatted string or stream. `encode_json` will work for all types that
have [json_type_traits](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_type_traits.md) defined.

```c++
template <class T, class CharT>
void encode_json(const T& val, 
                 std::basic_string<CharT>& s, 
                 indenting line_indent); // (1)

template <class T, class CharT>
void encode_json(const T& val,
                 std::basic_string<CharT>& s, 
                 const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(), 
                 indenting line_indent = indenting::no_indent); // (2)

template <class T, class CharT>
void encode_json(const T& val, 
                 std::basic_ostream<CharT>& os, 
                 indenting line_indent); // (3)

template <class T, class CharT>
void encode_json(const T& val,
                 std::basic_ostream<CharT>& os, 
                 const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(), 
                 indenting line_indent = indenting::no_indent); // (4)

template <class T, class CharT>
void encode_json(const T& val, 
                 basic_json_content_handler<CharT>& encoder); // (5)

template <class T, class CharT, class ImplementationPolicy, class Allocator>
void encode_json(const T& val,
                 std::basic_string<CharT>& s, 
                 const basic_json_encode_options<CharT>& options, 
                 indenting line_indent,
                 const basic_json<CharT,ImplementationPolicy,Allocator>& context_j); // (6)

template <class T, class CharT, class ImplementationPolicy, class Allocator>
void encode_json(const T& val,
                 std::basic_ostream<CharT>& os, 
                 const basic_json_encode_options<CharT>& options, 
                 indenting line_indent,
                 const basic_json<CharT,ImplementationPolicy,Allocator>& context_j); // (7)

template <class T, class CharT, class ImplementationPolicy, class Allocator>
void encode_json(const T& val,
                 basic_json_content_handler<CharT>& encoder,
                 const basic_json<CharT, ImplementationPolicy, Allocator>& context_j); // (8)
```

(1) Encode `val` to string with the specified line indenting.

(2) Encode `val` to string with the specified options and line indenting.

(3) Encode `val` to output stream with the specified line indenting.

(4) Encode `val` to output stream with the specified options and line indenting.

(5) Convert `val` to json events and stream through content handler.

(6) Encode `val` to string with the specified options and line indenting,
    using the context object `context_j` as a prototype when encoding requires
    a temporary `basic_json` value.

(7) Encode `val` to output stream with the specified options and line indenting,
    using the context object `context_j` as a prototype when encoding requires
    a temporary `basic_json` value.

(8) Convert `val` to json events and stream through content handler,
    using the context object `context_j` as a prototype when encoding requires
    a temporary `basic_json` value.

Functions (6)-(8) only participate in overload resolution if `T` is not a `basic_json` type.  

#### Parameters

<table>
  <tr>
    <td>val</td>
    <td>C++ data structure</td> 
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

### See also

- [basic_json_content_handler](basic_json_content_handler.md)
- [basic_json_options](basic_json_options.md)
    
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
    
#### Contain JSON output in an object (prettified output)

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string, std::tuple<std::string, std::string, double>> employees =
    {
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    json_stream_encoder encoder(std::cout);

    encoder.begin_object();
    encoder.name("Employees");
    encode_json(employees, encoder);
    encoder.end_object();
    encoder.flush();
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
    
#### Contain JSON output in an object (compressed output)

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string, std::tuple<std::string, std::string, double>> employees =
    {
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    json_compressed_stream_encoder encoder(std::cout);

    encoder.begin_object();
    encoder.name("Employees");
    encode_json(employees, encoder);
    encoder.end_object();
    encoder.flush();
}
```
Output:
```json
{"Employees":{"Jane Doe":["Commission","Sales",20000.0],"John Smith":["Hourly","Software Engineer",10000.0]}}
```

### See also

- [basic_json_encoder](basic_json_encoder.md)  
- [decode_json](decode_json.md)


