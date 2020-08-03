### jsoncons Specializations

jsoncons supports many types in the standard library.

* integer types 
* float and double
* bool
* [nullptr_t](https://en.cppreference.com/w/cpp/types/nullptr_t) (since 0.155.0)
* [basic_string](#basic_string) - jsoncons supports [std::basic_string](https://en.cppreference.com/w/cpp/string/basic_string) 
with character types `char` and `wchar_t`
* [basic_string_view](#basic_string_view) - jsoncons supports [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view) 
with character types `char` and `wchar_t`
* [duration](#duration) (since 0.155.0) - covers [std::chrono::duration](https://en.cppreference.com/w/cpp/chrono/duration)
for tick periods `std::ratio<1>` (one second), `std::milli` and  `std::nano`.
* [pair](#pair)
* [tuple](#tuple)
* [optional](#optional)
* [shared_ptr and unique_ptr](#shared_ptr) - if `T` is a class that is not a polymorphic class (does not have any virtual functions),
jsoncons provides specializations for `std::shared_ptr<T>` and `std::unique_ptr<T>`
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
for tick periods `std::ratio<1>` (one second), `std::milli` and  `std::nano`.

#### CBOR example (integer)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>

using jsoncons::json;
namespace cbor = jsoncons::cbor;

int main()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(duration);

    std::vector<uint8_t> data;
    cbor::encode_cbor(time, data);

    /*
      c1, // Tag 1 (epoch time)
        1a, // 32 bit unsigned integer
          5f,23,29,18 // 1596139800
    */

    std::cout << "CBOR bytes:\n" << jsoncons::byte_string_view(data) << "\n\n";

    auto seconds = cbor::decode_cbor<std::chrono::seconds>(data);
    std::cout << "Time since epoch (seconds): " << seconds.count() << "\n";
}
```
Output:
```
CBOR bytes: 
c1,1a,5f,23,29,18

Time since epoch (seconds): 1596139800
```

#### CBOR example (double)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>

using jsoncons::json;
namespace cbor = jsoncons::cbor;

int main()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::duration<double>>(duration);

    std::vector<uint8_t> data;
    cbor::encode_cbor(time, data);

    /*
      c1, // Tag 1 (epoch time)
        fb,  // Double
          41,d7,c8,ca,46,1c,0f,87 // 1596139800.43845
    */

    std::cout << "CBOR bytes:\n" << jsoncons::byte_string_view(data) << "\n\n";

    auto seconds = cbor::decode_cbor<std::chrono::duration<double>>(data);
    std::cout << "Time since epoch (seconds): " << seconds.count() << "\n";

    auto milliseconds = cbor::decode_cbor<std::chrono::milliseconds>(data);
    std::cout << "Time since epoch (milliseconds): " << milliseconds.count() << "\n";
}
```
Output:
```
CBOR bytes:
c1,fb,41,d7,c8,ca,46,1c,0f,87

Time since epoch (seconds): 1596139800.43845
Time since epoch (milliseconds): 1596139800438
```

#### MessagePack example (timestamp 32)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <iostream>

using jsoncons::json;
namespace msgpack = jsoncons::msgpack;

int main()
{
    std::vector<uint8_t> data = {
        0xd6, 0xff, // timestamp 32
        0x5a,0x4a,0xf6,0xa5 // 1514862245
    };
    auto seconds = msgpack::decode_msgpack<std::chrono::seconds>(data);
    std::cout << "Seconds elapsed since 1970-01-01 00:00:00 UTC: " << seconds.count() << "\n";
}
```
Output:
```
Seconds elapsed since 1970-01-01 00:00:00 UTC: 1514862245
```

#### MessagePack example (timestamp 64)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <iostream>

using jsoncons::json;
namespace msgpack = jsoncons::msgpack;

int main()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto dur_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    std::vector<uint8_t> data;
    msgpack::encode_msgpack(dur_nano, data);

    /*
        d7, ff, // timestamp 64
        e3,94,56,e0, // nanoseconds in 30-bit unsigned int
        5f,22,b6,8b // seconds in 34-bit unsigned int         
    */ 

    std::cout << "MessagePack bytes:\n" << jsoncons::byte_string_view(data) << "\n\n";

    auto nanoseconds = msgpack::decode_msgpack<std::chrono::nanoseconds>(data);
    std::cout << "nanoseconds elapsed since 1970-01-01 00:00:00 UTC: " << nanoseconds.count() << "\n";

    auto milliseconds = msgpack::decode_msgpack<std::chrono::milliseconds>(data);
    std::cout << "milliseconds elapsed since 1970-01-01 00:00:00 UTC: " << milliseconds.count() << "\n";

    auto seconds = msgpack::decode_msgpack<std::chrono::seconds>(data);
    std::cout << "seconds elapsed since 1970-01-01 00:00:00 UTC: " << seconds.count() << "\n";
}
```
Output:
```
nanoseconds elapsed since 1970-01-01 00:00:00 UTC: 1596128821304212600
milliseconds elapsed since 1970-01-01 00:00:00 UTC: 1596128821304
seconds elapsed since 1970-01-01 00:00:00 UTC: 1596128821
```

#### MessagePack example (timestamp 96)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <iostream>

using jsoncons::json;
namespace msgpack = jsoncons::msgpack;

int main()
{
    std::vector<uint8_t> input = {
        0xc7,0x0c,0xff, // timestamp 96
        0x3b,0x9a,0xc9,0xff, // 999999999 nanoseconds in 32-bit unsigned int
        0xff,0xff,0xff,0xff,0x7c,0x55,0x81,0x7f // -2208988801 seconds in 64-bit signed int
    };

    auto milliseconds = msgpack::decode_msgpack<std::chrono::milliseconds>(input);
    std::cout << "milliseconds elapsed since 1970-01-01 00:00:00 UTC: " << milliseconds.count() << "\n";

    auto seconds = msgpack::decode_msgpack<std::chrono::seconds>(input);
    std::cout << "seconds elapsed since 1970-01-01 00:00:00 UTC: " << seconds.count() << "\n";
}
```
Output:
```
milliseconds elapsed since 1970-01-01 00:00:00 UTC: -2208988801999
seconds elapsed since 1970-01-01 00:00:00 UTC: -2208988801
```

#### BSON example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <iostream>

using jsoncons::json;
namespace bson = jsoncons::bson;

int main()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    json j;
    j.try_emplace("time", time);

    auto milliseconds = j["time"].as<std::chrono::milliseconds>();
    std::cout << "Time since epoch (milliseconds): " << milliseconds.count() << "\n\n";
    auto seconds = j["time"].as<std::chrono::seconds>();
    std::cout << "Time since epoch (seconds): " << seconds.count() << "\n\n";

    std::vector<uint8_t> data;
    bson::encode_bson(j, data);

    std::cout << "BSON bytes:\n" << jsoncons::byte_string_view(data) << "\n\n";

/*
    13,00,00,00, // document has 19 bytes
      09, // UTC datetime
        74,69,6d,65,00, // "time"
        ea,14,7f,96,73,01,00,00, // 1595957777642
    00 // terminating null    
*/
}
```
Output:
```
Time since epoch (milliseconds): 1595957777642

Time since epoch (seconds): 1595957777

BSON bytes:
13,00,00,00,09,74,69,6d,65,00,ea,14,7f,96,73,01,00,00,00
```

### pair

The pair specialization encodes an `std::pair` as a JSON array of size 2.

### tuple

The tuple specialization encodes an `std::tuple` as a fixed size JSON array.

#### Example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <cassert>
#include <iostream>

using qualifying_results_type = std::tuple<std::size_t,std::string,std::string,std::string,std::chrono::milliseconds>;

int main()
{
    std::vector<qualifying_results_type> results = {
        {1,"Lewis Hamilton","Mercedes","1'24.303",std::chrono::milliseconds(0)},
        {2,"Valtteri Bottas","Mercedes","1'24.616",std::chrono::milliseconds(313)},
        {3,"Max Verstappen","Red Bull","1'25.325",std::chrono::milliseconds(1022)}
    };

    std::string json_data;
    encode_json_pretty(results, json_data);
    std::cout << json_data << "\n\n";
    auto results1 = decode_json<std::vector<qualifying_results_type>>(json_data);
    assert(results1 == results);

    csv::csv_options csv_options;
    options2.column_names("Pos,Driver,Entrant,Time,Gap");
    options2.mapping(csv::mapping_kind::n_rows);
    std::string csv_data;
    options2.header_lines(1);
    csv::encode_csv(results, csv_data, csv_options);
    std::cout << csv_data << "\n\n";
    auto results2 = csv::decode_csv<std::vector<qualifying_results_type>>(csv_data, csv_options);
    assert(results2 == results);

    std::vector<uint8_t> bson_data;
    bson::encode_bson(results, bson_data);
    auto results3 = bson::decode_bson<std::vector<qualifying_results_type>>(bson_data);
    assert(results3 == results);

    std::vector<uint8_t> cbor_data;
    cbor::encode_cbor(results, cbor_data);
    auto results4 = cbor::decode_cbor<std::vector<qualifying_results_type>>(cbor_data);
    assert(results4 == results);

    std::vector<uint8_t> msgpack_data;
    msgpack::encode_msgpack(results, msgpack_data);
    auto results5 = msgpack::decode_msgpack<std::vector<qualifying_results_type>>(msgpack_data);
    assert(results5 == results);

    std::vector<uint8_t> ubjson_data;
    ubjson::encode_ubjson(results, ubjson_data);
    auto results6 = ubjson::decode_ubjson<std::vector<qualifying_results_type>>(ubjson_data);
    assert(results6 == results);
}

```
Output:
```
[
    [1, "Lewis Hamilton", "Mercedes", "1'24.303", 0],
    [2, "Valtteri Bottas", "Mercedes", "1'24.616", 313],
    [3, "Max Verstappen", "Red Bull", "1'25.325", 1022]
]

Pos,Driver,Entrant,Time,Gap
1,Lewis Hamilton,Mercedes,1'24.303,0
2,Valtteri Bottas,Mercedes,1'24.616,313
3,Max Verstappen,Red Bull,1'25.325,1022
```

### variant

#### Example

```c++
int main()
{
    using variant_type = std::variant<std::nullptr_t, int, double, bool, std::string>;
    
    std::vector<variant_type> v = {nullptr, 10, 5.1, true, std::string("Hello World")}; 

    std::string buffer;
    jsoncons::encode_json_pretty(v, buffer);
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

