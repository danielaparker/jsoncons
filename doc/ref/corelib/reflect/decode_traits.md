### jsoncons::reflect::decode_traits

```cpp
#include <jsoncons/reflect/decode_traits.hpp>
```

<br>

`decode_traits` define a compile time template based interface for converting a stream of [staj_events](../staj_events.md) 
into a C++ data structure. They are used in the `decode_<format>` functions such as [decode_json](../decode_json.md). 

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





