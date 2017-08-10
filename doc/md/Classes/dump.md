```c++
jsoncons::dump
```
Serialize C++ object as a JSON formatted stream, governed by `json_stream_traits`

### Header
```c++
#include <jsoncons/json_stream_traits.hpp>

template <class CharT, class T>
void dump(const T& val, basic_json_output_handler<CharT>& handler)

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os)

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os)

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os, bool pprint)

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os, bool pprint)


```
### Parameters

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
    <td>pprint</td>
    <td><code>true</code> to pretty print, otherwise <code>false</code></td> 
  </tr>
</table>

### Return value

None
    
### Examples

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json_stream_traits.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    dump(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    dump(employees,std::cout,true);
}
```
```
Output:
(1)
{"Jane Doe":["Commission","Sales",20000.0],"John Smith":["Hourly","Software Engineer",10000.0]}

(2) Again, with pretty print
{
    "Jane Doe": ["Commission","Sales",20000.0],
    "John Smith": ["Hourly","Software Engineer",10000.0]
}
```

