### msgpack extension

The msgpack extension implements decode from and encode to the [MessagePack](http://msgpack.org/index.html) binary serialization format.

[encode_msgpack](encode_msgpack.md)

[decode_msgpack](decode_msgpack.md)

### Examples

Example file (book.json):
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
```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>

using namespace jsoncons;
using namespace jsoncons::msgpack;

int main()
{
    std::ifstream is("input/book.json");
    ojson j1;
    is >> j1;

    // Encode ojson to MessagePack
    std::vector<uint8_t> v;
    encode_msgpack(j1, v);

    // Decode MessagePack to ojson 
    ojson j2 = decode_msgpack<ojson>(v);

    std::cout << pretty_print(j2) << std::endl;

    // or to json (now alphabetically sorted)
    json j3 = decode_msgpack<json>(v);

    // or to wjson (converts from utf8 to wide characters)
    wjson j4 = decode_msgpack<wjson>(v);
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



