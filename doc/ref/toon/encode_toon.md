### jsoncons::toon::encode_toon

Encodes a `basic_json` value into the [toon-format](https://github.com/toon-format/toon) data format.

```cpp
#include <jsoncons_ext/toon/encode_toon.hpp>

template <typename T,typename StringLike>
void encode_toon(const T& jval, StringLike& cont,
    const toon_encode_options& options = toon_encode_options());         (1) (since 1.6.0)

template <typename T>
void encode_toon(const T& jval, std::ostream& os,
    const toon_encode_options& options = toon_encode_options());         (2) (since 1.6.0)
```

(1) Writes a value of type T into a byte container in the TOON data format, using the specified (or defaulted) [options](toon_options.md). 
Type 'T' must be an instantiation of [basic_json](../corelib/basic_json.md) 
or support jsoncons reflection traits. 
Type `BytesLike` must be back insertable and have member type `value_type` with size exactly 8 bits (since 0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.

(2) Writes a value of type T into a binary stream in the TOON data format, using the specified (or defaulted) [options](toon_options.md). 
Type 'T' must be an instantiation of [basic_json](../corelib/basic_json.md). 

### Examples

#### Encode to string

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/toon.hpp>
#include <iostream>

namespace toon = jsoncons::toon;

int main()
{
    std::string str = R"({
  "context": {
    "task": "Our favorite hikes together",
    "location": "Boulder",
    "season": "spring_2025"
  },
  "friends": ["ana", "luis", "sam"],
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
        )";
    auto j = jsoncons::ojson::parse(str);

    std::string buffer;
    toon::encode_toon(j, buffer);

    std::cout << buffer << "\n";
}
```
Output:
```
context:
  task: Our favorite hikes together
  location: Boulder
  season: spring_2025
friends[3]: ana,luis,sam
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true
```

Exanple credit: [toon-format](https://github.com/toon-format/toon)

### See also

[decode_toon](decode_toon.md)

