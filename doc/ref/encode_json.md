### jsoncons::encode_json

Encode a C++ data structure to a JSON formatted string or stream. `encode_json` will work for all types that
have [json_type_traits](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_type_traits.md) defined.

```c++
#include <jsoncons/encode_json.hpp>

template <class T, class Container>
void encode_json(const T& val,
                 Container& s, 
                 const basic_json_encode_options<Container::value_type>& options = 
                     basic_json_encode_options<Container::value_type>()); // (1)

template <class T, class CharT>
void encode_json(const T& val,
                 std::basic_ostream<CharT>& os, 
                 const basic_json_encode_options<CharT>& options = 
                     basic_json_encode_options<CharT>()); // (2)

template <class T, class Container>
void encode_json_pretty(const T& val,
                        Container& s, 
                        const basic_json_encode_options<Container::value_type>& options = 
                            basic_json_encode_options<Container::value_type>()); // (3) (since 0.155.0)

template <class T, class CharT>
void encode_json_pretty(const T& val,
                        std::basic_ostream<CharT>& os, 
                        const basic_json_encode_options<CharT>& options = 
                            basic_json_encode_options<CharT>()); // (4) (since 0.155.0)

template <class T, class CharT>
void encode_json(const T& val, 
                 basic_json_visitor<CharT>& encoder); // (5)

template <class T, class Container>
void encode_json(const T& val, 
                 Container& s, 
                 indenting line_indent); // (6)

template <class T, class Container>
void encode_json(const T& val,
                 Container& s, 
                 const basic_json_encode_options<Container::value_type>& options, 
                 indenting line_indent); // (7)

template <class T, class CharT>
void encode_json(const T& val, 
                 std::basic_ostream<CharT>& os, 
                 indenting line_indent); // (8)

template <class T, class CharT>
void encode_json(const T& val,
                 std::basic_ostream<CharT>& os, 
                 const basic_json_encode_options<CharT>& options, 
                 indenting line_indent); // (9)
```

(1) Encode `val` into a character container using the specified (or defaulted) [options](basic_json_options.md).

(2) Encode `val` to an output stream using the specified (or defaulted) [options](basic_json_options.md).

(3) Encode `val` into a character container using the specified (or defaulted) [options](basic_json_options.md)
and prettified with spaces and line breaks.

(4) Encode `val` to an output stream using the specified (or defaulted) [options](basic_json_options.md)
and prettified with spaces and line breaks.

(5) Convert `val` to json events and stream through encoder.

(6-9) Legacy overloads that indicate prettified output with [line_indent](indenting.md) parameter.

#### Parameters

<table>
  <tr>
    <td>val</td>
    <td>C++ data structure</td> 
  </tr>
  <tr>
    <td>visitor</td>
    <td>JSON output visitor</td> 
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
    using employee_collection = std::map<std::string,std::tuple<std::string,std::string,double>>;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    encode_json(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    encode_json_pretty(employees, std::cout);
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
    encoder.key("Employees");
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

    compact_json_stream_encoder encoder(std::cout);

    encoder.begin_object();
    encoder.key("Employees");
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

[basic_json_visitor](basic_json_visitor.md)  

[basic_json_options](basic_json_options.md)  

[basic_json_encoder](basic_json_encoder.md)  

[decode_json](decode_json.md)  

