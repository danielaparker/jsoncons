### jsoncons::toon::decode_toon, try_decode_toon

Decodes a toon-format to a `basic_json`.

```cpp
#include <jsoncons/decode_toon.hpp>

template <typename T,typename StringViewLike>
T decode_toon(const StringViewLike& s,
    const toon_decode_options& options 
        = toon_decode_options());                                  (1) (since 1.6.0)

template <typename T>
T decode_toon(std::istream& is,
    const toon_decode_options& options = toon_decode_options());   (2) (since 1.6.0)

template <typename T,typename StringViewLike>
read_result<T> try_decode_toon(const StringViewLike& s,
    const toon_decode_options& options 
        = toon_decode_options());                                  (3) (since 1.6.0)

template <typename T>
read_result<T> try_decode_toon(std::istream& is,
    const toon_decode_options& options = toon_decode_options());   (4) (since 1.6.0)
```

(1) Reads JSON from a contiguous character sequence provided by `s` into a type T, using the specified (or defaulted) [options](basic_json_options.md). 
Type 'T' must be an instantiation of [basic_json](basic_json.md) 
or support [json_type_traits](../json_type_traits/json_type_traits.md).

(2) Reads JSON from an input stream into a type T, using the specified (or defaulted) [options](basic_json_options.md). 
Type 'T' must be an instantiation of [basic_json](basic_json.md) 
or support [json_type_traits](../json_type_traits/json_type_traits.md).

(3)-(4) Non-throwing versions of (1)-(2)

#### Parameters

<table>
  <tr>
    <td>s</td>
    <td>Character sequence</td> 
  </tr>
  <tr>
    <td>is</td>
    <td>Input stream</td> 
  </tr>
  <tr>
    <td>options</td>
    <td>Deserialization options</td> 
  </tr>
</table>

#### Return value

(1)-(5) Deserialized value

(6)-(10) [read_result<T>](read_result.md)

#### Exceptions

(1)-(5) Throw [ser_error](ser_error.md) if decode fails.

Any overload may throw `std::bad_alloc` if memory allocation fails.

### Examples

#### Decode from string, non-throwing overload

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/decode_toon.hpp>
#include <iostream>

namespace toon = jsoncons::toon;

int main()
{
    std::string toon_str = R"(context:
  task: Our favorite hikes together
  location: Boulder
  season: spring_2025
friends[3]: ana,luis,sam
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true)";

    try
    {
        toon::decode_toon<jsoncons::ojson>(toon_str);
        std::cout << pretty_print(*result) << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}
```
Output:

```json
{
    "context": {
        "task": "Our favorite hikes together",
        "location": "Boulder",
        "season": "spring_2025"
    },
    "friends": [
        "ana",
        "luis",
        "sam"
    ],
    "hikes": [
        {
            "id": 1,
            "name": "Blue Lake Trail",
            "distanceKm": 7.5,
            "elevationGain": 320,
            "companion": "ana",
            "wasSunny": true
        },
        {
            "id": 2,
            "name": "Ridge Overlook",
            "distanceKm": 9.2,
            "elevationGain": 540,
            "companion": "luis",
            "wasSunny": false
        },
        {
            "id": 3,
            "name": "Wildflower Loop",
            "distanceKm": 5.1,
            "elevationGain": 180,
            "companion": "sam",
            "wasSunny": true
        }
    ]
}
```

#### Decode from stream, non-throwing overload

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/decode_toon.hpp>
#include <sstream>
#include <iostream>

namespace toon = jsoncons::toon;

int main()
{
        std::string toon_str = R"(orders[2]:
  - orderId: ORD-001
    customer:
      name: Alice Chen
      email: alice@example.com
    items[2]{sku,quantity,price}:
      WIDGET-A,2,29.99
      GADGET-B,1,49.99
    total: 109.97
    status: shipped
  - orderId: ORD-002
    customer:
      name: Bob Smith
      email: bob@example.com
    items[1]{sku,quantity,price}:
      THING-C,3,15
    total: 45
    status: delivered)";

    std::stringstream is(toon_str);
    auto result = toon::try_decode_toon<jsoncons::ojson>(is);
    if (!result)
    {
        std::cerr << result.error().message();
        exit(1);
    }
    std::cout << pretty_print(*result) << "\n";
}
```

Output:
```json
{
    "orders": [
        {
            "orderId": "ORD-001",
            "customer": {
                "name": "Alice Chen",
                "email": "alice@example.com"
            },
            "items": [
                {
                    "sku": "WIDGET-A",
                    "quantity": 2,
                    "price": 29.99
                },
                {
                    "sku": "GADGET-B",
                    "quantity": 1,
                    "price": 49.99
                }
            ],
            "total": 109.97,
            "status": "shipped"
        },
        {
            "orderId": "ORD-002",
            "customer": {
                "name": "Bob Smith",
                "email": "bob@example.com"
            },
            "items": [
                {
                    "sku": "THING-C",
                    "quantity": 3,
                    "price": 15
                }
            ],
            "total": 45,
            "status": "delivered"
        }
    ]
}
```

Example credit: [toon-format](https://github.com/toon-format/toon)

### See also

[encode_json](encode_json.md)


