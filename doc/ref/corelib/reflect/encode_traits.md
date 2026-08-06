### jsoncons::reflect::encode_traits

```cpp
#include <jsoncons/reflect/encode_traits.hpp>
```

<br>

`encode_traits` define a compile time template based interface for converting 
a C++ data structure into a character buffer or an output stream. They are used in the 
`encode_<format>` functions such as [encode_json](./encode_json.md).

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





