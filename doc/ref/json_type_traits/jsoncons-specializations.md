### jsoncons Specializations

jsoncons supports most types in the standard library.

* [basic_string](#basic_string) - jsoncons supports [std::basic_string](https://en.cppreference.com/w/cpp/string/basic_string) 
with character types `char` and `wchar_t`
* [basic_string_view](#basic_string_view) - jsoncons supports [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view) 
with character types `char` and `wchar_t`
* [duration](#duration) - jsoncons supports [std::chrono::duration](https://en.cppreference.com/w/cpp/chrono/duration)
with a tick period of 1 second.
* [pair](#pair)
* [tuple](#tuple)
* [optional](#optional)
* [shared_ptr and unique_ptr](#shared_ptr) - if `T` is a class that is not a polymorphic class, i.e., does not have any virtual functions,
jsoncons provides specializations for [std::shared_ptr<T>](https://en.cppreference.com/w/cpp/memory/unique_ptr) and 
[std::unique_ptr<T>](https://en.cppreference.com/w/cpp/memory/unique_ptr) 
* [variant](#variant)
* [sequence containers](#sequence) - includes [std::array](https://en.cppreference.com/w/cpp/container/array), 
[std::vector](https://en.cppreference.com/w/cpp/container/vector), [std::deque](https://en.cppreference.com/w/cpp/container/deque), 
[std::forward_list](https://en.cppreference.com/w/cpp/container/forward_list) and [std::list](https://en.cppreference.com/w/cpp/container/list).
* [associative containers](#associative) - includes [std::set](https://en.cppreference.com/w/cpp/container/set), 
[std::map](https://en.cppreference.com/w/cpp/container/map), [std::multiset](https://en.cppreference.com/w/cpp/container/multiset), 
and [std::multimap](https://en.cppreference.com/w/cpp/container/multimap).
* [unordered associative containers](#unordered) - includes unordered associative containers
[std::unordered_set](https://en.cppreference.com/w/cpp/container/unordered_set), 
[std::unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map), 
[std::unordered_multiset](https://en.cppreference.com/w/cpp/container/unordered_multiset), and 
[std::unordered_multimap](https://en.cppreference.com/w/cpp/container/unordered_multimap).

### duration

includes [std::chrono::duration](https://en.cppreference.com/w/cpp/chrono/duration)
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

    /*
        c1 -- tag value 1 (seconds relative to 1970-01-01T00:00Z in UTC time) 
        fb -- double precision float
        41,d7,c6,7b,8f,c7,05,51 -- 1595534911.1097 
    */
}
```
Output:
```
Time since epoch: 1595534911.1097

CBOR bytes: c1,fb,41,d7,c6,7b,8f,c7,05,51
```

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


