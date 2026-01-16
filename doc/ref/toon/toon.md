### toon extension

The toon extension implements encode to and decode from the [TOON format](http://toon.org/index.html) data format.
You can either parse into or serialize from a variant-like data structure, [basic_json](../corelib/basic_json.md), or your own
data structures, using [json_type_traits](../corelib/json_type_traits/json_type_traits.md).

[decode_toon, try_decode_toon](decode_toon.md)

[basic_toon_cursor](basic_toon_cursor.md)

[encode_toon](encode_toon.md)

[basic_toon_encoder](basic_toon_encoder.md)

[toon_options](toon_options.md)

#### Mappings between MessagePack and jsoncons data items

MessagePack data item                              |ext type | jsoncons data item|jsoncons tag  
-------------------------------------------------- |-----------------|---------------|------------------
 nil                                               |                  | null          |                  
 true, false                                     |                  | bool          |                  
 negative fixnum, int 8, int 16, int 32, int 64    |                  | int64         |                  
 positive fixnum, uint 8, uint 16, uint 32, uint 64|                  | uint64        |                  
 float32, float64                                |                  | double        |                  
 fixstr, str 8, str 16, str 32                   |                  | string        |                  
 bin 8, bin 16, bin 32                           |                  | byte_string   |                  
 fixext1, fixext2, fixext4, fixext8, fixext16, ext8, ext16, ext32    |0-127| byte_string               |
 4 byte length   |-1 (timestamp 32)                |uint64| seconds
 8 byte length   |-1 (timestamp 64)                |string| epoch_nanosecond
 12 byte length  |-1 (timestamp 96)               |string| epoch_nanosecond
 array                                             |                  | array         |                  
 map                                               |                  | object        |                  

### Examples

#### ext format code example

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/toon.hpp>
#include <cassert>

using namespace jsoncons;

int main()
{
    std::vector<uint8_t> input = {

        0x82, // map, length 2
          0xa5, // string, length 5
            'H','e','l','l','o',
          0xa5, // string, length 5
            'W','o','r','l','d',
          0xa4, // string, length 4
             'D','a','t','a',
          0xc7, // ext8 format code
            0x06, // length 6
            0x07, // type
              'f','o','o','b','a','r'
    };

    ojson j = toon::decode_toon<ojson>(input);

    std::cout << "(1)\n" << pretty_print(j) << "\n\n";
    std::cout << "(2) " << j["Data"].tag() << "("  << j["Data"].ext_tag() << ")\n\n";
    
    // Get ext value as a std::vector<uint8_t>
    auto v = j["Data"].as<std::vector<uint8_t>>(); 

    std::cout << "(3)\n";
    std::cout << byte_string_view(v) << "\n\n";

    std::vector<uint8_t> output;
    toon::encode_toon(j,output);
    assert(output == input);
}
```
Output:
```
(1)
{
    "Hello": "World",
    "Data": "Zm9vYmFy"
}

(2) ext(7)

(3)
66,6f,6f,62,61,72
```

#### JSON to message pack

Input JSON file `book.json`

```json
[
    {
        "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
    },
    {
        "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
    }
]
```
```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/toon.hpp>

using namespace jsoncons;

int main()
{
    std::ifstream is("input/book.json");
    ojson j1;
    is >> j1;

    // Encode ojson to MessagePack
    std::vector<uint8_t> v;
    toon::encode_toon(j1, v);

    // Decode MessagePack to ojson 
    ojson j2 = toon::decode_toon<ojson>(v);

    std::cout << pretty_print(j2) << '\n';

    // or to json (now alphabetically sorted)
    json j3 = toon::decode_toon<json>(v);

    // or to wjson (converts from utf8 to wide characters)
    wjson j4 = toon::decode_toon<wjson>(v);
}
```
Output:
```json
[
    {
        "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
    },
    {
        "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
    }
]
```



