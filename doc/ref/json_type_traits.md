### jsoncons::json_type_traits

```c++
#include <jsoncons/json_type_traits.hpp>
```

<br>

`json_type_traits` defines a compile time template based interface for conversion between a `basic_json` value
and a value of some other type.

The interface that `json_type_traits` specializations must conform to is as follows:

```c++
template <class Json, class T, class Enable=void>
struct JsonTypeTraits
{
    using allocator_type = typename Json::allocator_type;

    static constexpr bool is(const Json&);

    static T as(const Json&);

    static Json to_json(const T&);

    static Json to_json(const T&, const allocator_type& alloc);
};
```

[jsoncons Specializations](json_type_traits/jsoncons-specializations.md)

[Convenience Macros](json_type_traits/convenience-macros.md)

[Custom Specializations](json_type_traits/custom-specializations.md)


