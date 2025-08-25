### jsoncons::encode_json, jsoncons::encode_json_pretty, jsoncons::try_encode_json, jsoncons::try_encode_json_pretty

Encode a C++ data structure to a JSON formatted string or stream. `encode_json` will work for all types that
have json reflection traites defined.

```cpp
#include <jsoncons/encode_json.hpp>

template <typename T,typename CharT>                                            (1)
void encode_json(const T& val, basic_json_visitor<CharT>& encoder);              

template <typename T,typename CharContainer>
void encode_json(const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (2)
        = basic_json_encode_options<CharContainer::value_type>();               

template <typename T,typename CharT>                                             
void encode_json(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (3)
        = basic_json_encode_options<CharT>();                                    

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc>
void encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (4) (since 0.171.0)
        = basic_json_encode_options<CharContainer::value_type>();               

template <typename T,typename CharT,typename Alloc,typename TempAlloc>                                             
void encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (5) (since 0.171.0)
        = basic_json_encode_options<CharT>());                                  

template <typename T,typename CharContainer>
void encode_json_pretty(const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (6) (since 0.155.0)
        = basic_json_encode_options<CharContainer::value_type>());              

template <typename T,typename CharT>
void encode_json_pretty(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (7) (since 0.155.0)        
        = basic_json_encode_options<CharT>());                                  

template <typename T,typename CharT>
write_result try_encode_json(const T& val, basic_json_visitor<CharT>& encoder); (8) (since 1.4.0)

template <typename T,typename CharContainer>
void encode_json_pretty(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (9) (since 1.4.0)
        = basic_json_encode_options<CharContainer::value_type>());              

template <typename T,typename CharT>
void encode_json_pretty(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (10) (since 1.4.0)
        = basic_json_encode_options<CharT>());                                          

template <typename T,typename CharContainer>
write_result try_encode_json(const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (11) (since 1.4.0)
        = basic_json_encode_options<CharContainer::value_type>();               

template <typename T,typename CharT>                                             
write_result try_encode_json(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (12) (since 1.4.0)
        = basic_json_encode_options<CharT>();                                   

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc>
write_result try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (13) (since 1.4.0)
        = basic_json_encode_options<CharContainer::value_type>();               

template <typename T,typename CharT,typename Alloc,typename TempAlloc>                                             
write_result try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (14) (since 1.4.0)
        = basic_json_encode_options<CharT>());                                  

template <typename T,typename CharContainer>
write_result try_encode_json_pretty(const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (15) (since 1.4.0)
        = basic_json_encode_options<CharContainer::value_type>());              

template <typename T,typename CharT>
write_result try_encode_json_pretty(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (16) (since 1.4.0)
        = basic_json_encode_options<CharT>());                                  

template <typename T,typename CharContainer>
write_result try_encode_json_pretty(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<CharContainer::value_type>& options         (17) (since 1.4.0)
        = basic_json_encode_options<CharContainer::value_type>());              

template <typename T,typename CharT>
write_result try_encode_json_pretty(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options                             (18) (since 1.4.0)
        = basic_json_encode_options<CharT>());                                          
```

(1) Write val to an encoder

(2) Encode `val` to a character container with "minified" output (single line) using the specified (or defaulted) [options](basic_json_options.md).

(3) Encode `val` to an output stream in "minified" output using the specified (or defaulted) [options](basic_json_options.md).

Functions (4)-(5) are identical to (2)-(3) except an [allocator_set](allocator_set.md) is passed as an additional argument.

(6) Encode `val` to a character container with "prettified" output (line indentation) using the specified (or defaulted) [options](basic_json_options.md).

(7) Encode `val` to an output stream with "prettified" output using the specified (or defaulted) [options](basic_json_options.md).

Functions (8)-(9) are identical to (6)-(7) except an [allocator_set](allocator_set.md) is passed as an additional argument.

(10)-(18) Non-throwing versions of (1)-(9)


#### Parameters

<table>
  <tr>
    <td>aset</td>
    <td>[allocator_set](../allocator_set)</td> 
  </tr>
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
</table>

#### Return value

(1)-(9) None 
    
(10)-(18) [write_result](write_result.md)  

#### Exceptions

(1)-(9) Throw [ser_error](ser_error.md) if encode fails.

Any overload may throw `std::bad_alloc` if memory allocation fails.
    
### Examples

#### Map with string-tuple pairs

```cpp
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

    std::cout << "(1)\n" << '\n'; 
    encode_json(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << '\n'; 

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

```cpp
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

```cpp
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

