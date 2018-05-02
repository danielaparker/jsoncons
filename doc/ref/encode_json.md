### jsoncons::encode_json

Serializes a C++ object to a JSON formatted string or stream. `encode_json` attempts to 
perform the conversion by streaming using `json_convert_traits`, and if
streaming is not supported, falls back to using `json_type_traits`. `encode_json` will work for all types that
have `json_type_traits` defined.

#### Header
```c++
#include <jsoncons/json.hpp>

template <class T, class CharT>
void encode_json(const T& val, basic_json_output_handler<CharT>& handler); // (1)

template <class T, class CharT>
void encode_fragment(const T& val, basic_json_output_handler<CharT>& handler); // (2)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os); // (3)

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
                 std::basic_ostream<CharT>& os); // (4)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, indenting line_indent); // (5)

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
                 std::basic_ostream<CharT>& os, indenting line_indent); // (6)
```

(1) Calls `begin_json()` on `handler`, applies `json_convert_traits` to serialize `val` to JSON output stream, and calls `end_json()` on `handler`.

(2) Applies `json_convert_traits` to serialize `val` to JSON output stream, but does not call begin_json() and `end_json()`.

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

- [json_output_handler](json_output_handler.md)
- [serialization_options](serialization_options.md)
    
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

    std::string s;
    jsoncons::encode_json(employees, s, jsoncons::indenting::indent);
    std::cout << "(1)\n" << s << std::endl;
    auto employees2 = jsoncons::decode_json<employee_collection>(s);

    std::cout << "\n(2)\n";
    for (const auto& pair : employees2)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
```
Output:
```
(1)
{
    "Jane Doe": ["Commission","Sales",20000.0],
    "John Smith": ["Hourly","Software Engineer",10000.0]
}

(2)
Jane Doe: Sales
John Smith: Software Engineer
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

    serializer.begin_json();       
    serializer.begin_object();       
    serializer.name("Employees");       
    encode_fragment(employees, serializer);
    serializer.end_object();       
    serializer.end_json();       
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
    
#### Extending `json_convert_traits`

```c++
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json.hpp>
#include <sstream>

using boost::numeric::ublas::matrix;

namespace jsoncons
{
    template <>
    struct json_convert_traits<matrix<double>>
    {
        static void encode(const matrix<double>& val, json_output_handler& handler)
        {
            handler.begin_array();
            for (size_t i = 0; i < val.size1(); ++i)
            {
                handler.begin_array();
                for (size_t j = 0; j < val.size2(); ++j)
                {
                    handler.double_value(val(i, j));
                }
                handler.end_array();
            }
            handler.end_array();
        }
    };
};

using namespace jsoncons;

int main()
{
    std::ostringstream oss;

    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    encode_json(A, oss, jsoncons::indenting::indent);

    std::cout << oss.str() << std::endl;
}
```
Output:
```json
[
    [1.0,2.0],
    [3.0,4.0]
]
```


