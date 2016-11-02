```c++
jsoncons::ojson

typedef basic_json<char,
                   JsonTraits = ojson_traits<char>,
                   Allocator = std::allocator<char>> ojson
```
The `ojson` class is an instantiation of the `basic_json` class template that uses `char` as the character type. The supplied `JsonTraits` template parameter keeps object members in their original order. 

The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header

    #include <jsoncons/json.hpp>

### Interface

The interface is the same as [json](json), with these provisos:

- In `ojson`, `object_allocator` is a random access iterator, so object members can be accessed by position, e.g.

```
ojson o = ojson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W"
}
)");

ojson::member_type member = o.members().begin()[1];
std::cout << member.name() << "=" << member.value() << std::endl;
```
- `ojson`, like `json`, supports object member `set` methods that take an `object_iterator` as the first parameter. But while with `json` that parameter is just a hint that allows optimization, with `ojson` it is the actual location where to insert the member.

- In `ojson`, the `set` members that just take a name and a value always insert the member at the end.

### See also

- [json](json) constructs a json value that sorts name-value members alphabetically

- [wjson](wjson) constructs a wide character json value that sorts name-value members alphabetically

- [owjson](owjson) constructs a wide character json value that retains the original name-value insertion order

## Examples
```c++
ojson o = ojson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W",
    "city" : "Toronto",
    "country" : "Canada"
}
)");

std::cout << pretty_print(o) << std::endl;
```
Output:
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada"
}
```
Insert "postal_code" at end
```c++
o.set("postal_code", "M5H 2N2");

std::cout << pretty_print(o) << std::endl;
```
Output:
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```
Insert "province" before "country"
```c++
auto it = o.find("country");
o.set(it,"province","Ontario");

std::cout << pretty_print(o) << std::endl;
```
Output:
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "province": "Ontario",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```
