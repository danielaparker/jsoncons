### jsoncons::reflect::json_conv_traits

```cpp
#include <jsoncons/reflect/json_conv_traits.hpp>
```

<br>

`json_conv_traits` defines a compile time template based interface for conversion between a `basic_json` value
and a value of some other type. `json_conv_traits` implementations must specialize a traits class for a type `T`:

```cpp
template <typename Json,typename T,typename Enable=void>
struct json_conv_traits
{
    using result_type = conversion_result<T>;

    static constexpr bool is(const Json& j) noexcept;

    template<typename Alloc,typename TempAlloc>
    static result_type try_as(const allocator_set<Alloc,TempAlloc>&, const Json& j);

    template <typename Alloc, typename TempAlloc>
    static Json to_json(const allocator_set<Alloc,TempAlloc>& aset, const T& val);
};
```

The function 

    json_conv_traits<Json,T>::is(const Json& j) noexcept

indictates whether `j` satisfies the requirements of type `T`. This function supports 
the type selection strategy when converting a `Json` value to the proper derived class 
in the polymorphic case, and when converting a `Json` value to the proper alternative 
type in the variant case.  

The function 

    template <typename Alloc, typename TempAlloc>
    T try_as(const allocator_set<Alloc,TempAlloc>& aset, const Json& j) 

tries to convert `j` to a value of type `T`.

The function 

    template <typename Alloc, typename TempAlloc>
    Json to_json(const allocator_set<Alloc,TempAlloc>& aset, const T& val)

tries to convert `val` into a `Json` value.

jsoncons includes specializiations for most types in the standard library. And it includes convenience 
macros that make specializing `json_conv_traits` for your own types easier.

