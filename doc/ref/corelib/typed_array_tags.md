### jsoncons::typed_array_tags

```cpp
#include <jsoncons/typed_array.hpp>

enum class typed_array_tags
{
    uint8 = 1,
    uint16 = 2,
    uint32 = 4,
    uint64 = 8,
    int8 = 16,
    int16 = 32,
    int32 = 64,
    int64 = 128, 
    half_float = 256, 
    float32 = 512, 
    float64 = 1024
};
```
Represents typed array element types.

`typed_array_tags` satisfies the requirements of a
[BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). This 
allows us to write e.g.

```cpp
constexpr auto mask = typed_array_tags::uint8 | typed_array_tags::uint16 | 
    typed_array_tags::uint32 | typed_array_tags::uint64;

if ((array_tag & mask) != typed_array_tags{}) 
{
    // Process arrays of unsigned integers
}
```  

