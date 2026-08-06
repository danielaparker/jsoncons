### jsoncons::reflect::decode_traits

```cpp
#include <jsoncons/reflect/decode_traits.hpp>
```

<br>

`json_decode` defines a compile time template based interface for streaming
JSON and other data formats. `json_traits` implementations must specialize 
a traits class for a type `T`,

```cpp
template <typename T,typename Enable = void>
struct decode_traits
{
    using value_type = T;
    using result_type = read_result<value_type>;
    
    template <typename Alloc,typename TempAlloc,typename CharT>
    static result_type decode(const allocator_set<Alloc,TempAlloc>& aset,
        basic_staj_cursor<CharT>& cursor);
};
```





