### jsoncons::encode_json

Serializes a C++ object to a JSON formatted string or stream. `encode_json` attempts to 
perform the conversion by streaming using `json_convert_traits`, and if
streaming is not supported, falls back to using `json_type_traits`. `encode_json` will work for all types that
have `json_type_traits` defined.

#### Header
```c++
#include <jsoncons/json.hpp>

template <class T, class CharT>
void encode_json(const T& val, basic_json_content_handler<CharT>& handler); // (1)

template <class T, class CharT>
void encode_fragment(const T& val, basic_json_content_handler<CharT>& handler); // (2)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os); // (3)

template <class T, class CharT>
void encode_json(const T& val, 
                 const basic_json_serializing_options<CharT>& options,
                 std::basic_ostream<CharT>& os); // (4)

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, indenting line_indent); // (5)

template <class T, class CharT>
void encode_json(const T& val, 
                 const basic_json_serializing_options<CharT>& options,
                 std::basic_ostream<CharT>& os, indenting line_indent); // (6)
```

(1) Calls `begin_document()` on `handler`, applies `json_convert_traits` to serialize `val` to JSON output stream, and calls `end_document()` on `handler`.

(2) Applies `json_convert_traits` to serialize `val` to JSON output stream, but does not call begin_document() and `end_document()`.

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
    template<>
    struct json_convert_traits<matrix<double>>
    {
        template <class CharT>
        static matrix<double> decode(std::basic_istringstream<CharT>& is,
                                     const basic_json_serializing_options<CharT>& options)
        {
            basic_json<CharT> j = basic_json<CharT>::parse(is, options);
            if (j.is_array() && j.size() > 0)
            {
                size_t m = j.size();
                size_t n = 0;
                for (const auto& a : j.array_range())
                {
                    if (a.size() > n)
                    {
                        n = a.size();
                    }
                }

                boost::numeric::ublas::matrix<double> A(m,n,double());
                for (size_t i = 0; i < m; ++i)
                {
                    const auto& a = j[i];
                    for (size_t j = 0; j < a.size(); ++j)
                    {
                        A(i,j) = a[j].template as<double>();
                    }
                }
                return A;
            }
            else
            {
                boost::numeric::ublas::matrix<double> A;
                return A;
            }
        }

        static void encode(const matrix<double>& val, json_content_handler& handler)
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

#### See also

- [decode_json](decode_json.md)


