### jsoncons Specializations

jsoncons supports most types in the standard library.

* [std::duration](#duration) - jsoncons supports [std::duration](https://en.cppreference.com/w/cpp/chrono/duration)
with a tick period of 1 second.

### duration

jsoncons supports [std::duration](https://en.cppreference.com/w/cpp/chrono/duration)
with a tick period of 1 second.

#### MessagePack example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <iostream>

using jsoncons::json;
namespace msgpack = jsoncons::msgpack;

int main()
{
    std::vector<uint8_t> data = {
        0xd6, // fixext 4 stores an integer and a byte array whose length is 4 bytes
        0xff, // timestamp
        0x5a,0x4a,0xf6,0xa5 // 1514862245
    };
    auto j = msgpack::decode_msgpack<json>(data);
    auto seconds = j.as<std::chrono::duration<uint32_t>>();
    std::cout << "Seconds elapsed since 1970-01-01 00:00:00 UTC: " << seconds.count() << "\n";
}
```
Output:
```
Seconds elapsed since 1970-01-01 00:00:00 UTC: 1514862245
```

#### CBOR example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>

using jsoncons::json;
namespace msgpack = jsoncons::msgpack;

int main()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    double time = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();

    json j(time, jsoncons::semantic_tag::epoch_time);

    auto dur = j.as<std::chrono::duration<double>>();
    std::cout << "Time since epoch: " << dur.count() << "\n\n";

    std::vector<uint8_t> data;
    cbor::encode_cbor(j, data);

    std::cout << "CBOR bytes: " << jsoncons::byte_string_view(data) << "\n\n";
}
```
Output:
```
Time since epoch: 1595534911.1097

CBOR bytes: c1,fb,41,d7,c6,7b,8f,c7,05,51
```

`T`|`j.is<T>()`|`j.as<T>()`|j is assignable from `T`
--------|-----------|--------------|---
`Json`|`true`|self|<em>&#x2713;</em>
`Json::object`|`true` if `j.is_object()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`Json::array`|`true` if `j.is_array()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`bool`|`true` if `j.is_bool()`, otherwise `false`|as `bool`|<em>&#x2713;</em>
`null_type`|`true` if `j.is_null()`, otherwise `false`|`null_type()` value if j.is_null(), otherwise throws|<em>&#x2713;</em>
`integral types`|`true` if `j.is_int64()` or `j.is_uint64()` and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`floating point types`|`true` if j.is_double() and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`std::basic_string<CharT>`<sup>1</sup>|`true` if `j.is<std::basic_string<CharT>>()`, otherwise `false`|j.as<std::basic_string<CharT>>|<em>&#x2713;</em>
`jsoncons::basic_string_view<CharT>`<sup>1</sup><sup>,2</sup>|`true` if `j.is<jsoncons::basic_string_view<CharT>>()`, otherwise `false`|j.as<std::basic_string_view<CharT>>|<em>&#x2713;</em>
STL sequence container (other than string) e.g. std::vector|`true` if array and each value is assignable to a `Json` value, otherwise `false`|if array and each value is convertible to `value_type`, as container, otherwise throws|<em>&#x2713;</em>
STL associative container e.g. `std::map<K,U>`|`true` if object and each `mapped_type` is assignable to `Json`, otherwise `false`|if object and each member value is convertible to `mapped_type`, as container|<em>&#x2713;</em>
`std::tuple<Args...>`|`true` if `j.is_array()` and each array element is assignable to the corresponding `tuple` element, otherwise false|tuple with array elements converted to tuple elements|<em>&#x2713;</em>
`std::pair<U,V>`|`true` if `j.is_array()` and `j.size()==2` and each array element is assignable to the corresponding pair element, otherwise false|pair with array elements converted to pair elements|<em>&#x2713;</em>
`std::shared_ptr<U>`<sup>3</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty shared_ptr if `j.is_null()`, otherwise `make_shared(j.as<U>())`|<em>&#x2713;</em>
`std::unique_ptr<U>`<sup>4</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty unique_ptr if `j.is_null()`, otherwise `make_unique(j.as<U>())`|<em>&#x2713;</em>
`jsoncons::optional<U>`<sup>5</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty `jsoncons::optional<U>` if `j.is_null()`, otherwise `jsoncons::optional<U>(j.as<U>())`|<em>&#x2713;</em>
`std::variant<Types...>`<sup>6</sup>|&nbsp;|<em>&#x2713;</em>
  
1. For `CharT` `char` or `wchar_t`.
2. `jsoncons::basic_string_view<CharT>` is aliased to [std::basic_string_view<CharT>](https://en.cppreference.com/w/cpp/utility/optional) if 
jsoncons detects the presence of C++17, or if `JSONCONS_HAS_STD_STRING_VIEW` is defined.  
3. Defined if `U` is not a polymorphic class, i.e., does not have any virtual functions.  
4. Defined if `U` is not a polymorphic class, i.e., does not have any virtual functions.   
5. `jsoncons::optional<U>` is aliased to [std::optional<U>](https://en.cppreference.com/w/cpp/utility/optional) if 
jsoncons detects the presence of C++17, or if `JSONCONS_HAS_STD_OPTIONAL` is defined.
6. Since v0.154.0  

### Examples

[Convert from and to standard library sequence containers](#A1)  
[Convert from and to standard library associative containers](#A2)  
[Convert from and to std::map with integer key](#A3)  
[Convert from and to std::tuple](#A4)  

<div id="A1"/> 

#### Convert from and to standard library sequence containers

```c++
std::vector<int> v{1, 2, 3, 4};
json j(v);
std::cout << "(1) "<< j << std::endl;
std::deque<int> d = j.as<std::deque<int>>();
```
Output:
```
(1) [1,2,3,4]
```

<div id="A2"/> 

#### Convert from and to standard library associative containers

```c++
std::map<std::string,int> m{{"one",1},{"two",2},{"three",3}};
json j(m);
std::cout << j << std::endl;
std::unordered_map<std::string,int> um = j.as<std::unordered_map<std::string,int>>();
```
Output:
```
{"one":1,"three":3,"two":2}
```

<div id="A3"/> 

#### Convert from and to std::map with integer key

```c++
std::map<short, std::string> m{ {1,"foo",},{2,"baz"} };

json j{m};

std::cout << "(1)\n";
std::cout << pretty_print(j) << "\n\n";

auto other = j.as<std::map<uint64_t, std::string>>();

std::cout << "(2)\n";
for (const auto& item : other)
{
    std::cout << item.first << " | " << item.second << "\n";
}
std::cout << "\n\n";
```
Output:

```c++
(1)
{
    "1": "foo",
    "2": "baz"
}

(2)
1 | foo
2 | baz
```

<div id="A4"/> 

#### Convert from and to std::tuple

```c++
    auto t = std::make_tuple(false,1,"foo");
    json j(t);
    std::cout << j << std::endl;
    auto t2 = j.as<std::tuple<bool,int,std::string>>();
```
Output:
```
[false,1,"foo"]
```


