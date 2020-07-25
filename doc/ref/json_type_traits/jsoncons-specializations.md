### jsoncons Specializations

jsoncons supports most types in the standard library.

* integer types 
* float and double
* bool
* [nullptr_t](https://en.cppreference.com/w/cpp/types/nullptr_t) (since 0.155.0)
* [basic_string](#basic_string) - jsoncons supports [std::basic_string](https://en.cppreference.com/w/cpp/string/basic_string) 
with character types `char` and `wchar_t`
* [basic_string_view](#basic_string_view) - jsoncons supports [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view) 
with character types `char` and `wchar_t`
* [duration](#duration) (since 0.155.0) - covers [std::chrono::duration](https://en.cppreference.com/w/cpp/chrono/duration)
with a tick period of 1 second.
* [pair](#pair)
* [tuple](#tuple)
* [optional](#optional)
* [shared_ptr and unique_ptr](#shared_ptr) - if `T` is a class that is not a polymorphic class (does not have any virtual functions),
jsoncons provides specializations for [std::shared_ptr<T>](https://en.cppreference.com/w/cpp/memory/unique_ptr) and 
[std::unique_ptr<T>](https://en.cppreference.com/w/cpp/memory/unique_ptr) 
* [variant](#variant) (since 0.154.0)
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

jsoncons supports [std::chrono::duration](https://en.cppreference.com/w/cpp/chrono/duration)
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

### pair

The pair specialization encodes an `std::pair` as a JSON array of size 2.

### tuple

The tuple specialization encodes an `std::tuple` as a fixed size JSON array.

#### Example

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

### variant

#### Example

```c++
int main()
{
    using variant_type = std::variant<std::nullptr_t, int, double, bool, std::string>;
    
    std::vector<variant_type> v = {nullptr, 10, 5.1, true, std::string("Hello World")}; 

    std::string buffer;
    jsoncons::encode_json(v, buffer, jsoncons::indenting::indent);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v2 = jsoncons::decode_json<std::vector<variant_type>>(buffer);

    auto visitor = [](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>)
                std::cout << "nullptr " << arg << '\n';
            else if constexpr (std::is_same_v<T, int>)
                std::cout << "int " << arg << '\n';
            else if constexpr (std::is_same_v<T, double>)
                std::cout << "double " << arg << '\n';
            else if constexpr (std::is_same_v<T, bool>)
                std::cout << "bool " << arg << '\n';
            else if constexpr (std::is_same_v<T, std::string>)
                std::cout << "std::string " << arg << '\n';
        };

    std::cout << "(2)\n";
    for (const auto& item : v2)
    {
        std::visit(visitor, item);
    }
}
```
Output:
```
(1)
[
    null,
    10,
    5.1,
    true,
    "Hello World"
]

(2)
nullptr nullptr
int 10
double 5.1
bool true
std::string Hello World
```

### sequence containers 

#### Example

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

### associative containers 

#### From std::map, to std::unordered_map

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
#### std::map with integer key

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

