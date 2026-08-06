### jsoncons::reflect::encode_traits

```cpp
#include <jsoncons/reflect/encode_traits.hpp>
```

<br>

`json_decode` defines a compile time template based interface for streaming
JSON and other data formats. `json_traits` implementations must specialize 
a traits class for a type `T`,

```cpp
template <typename T, typename Enable = void>
struct encode_traits
{
public:
    template <typename CharT, typename Alloc, typename TempAlloc>
    static write_result encode(const allocator_set<Alloc,TempAlloc>& aset,
        const T& val, 
        basic_json_visitor<CharT>& encoder);
};
```





