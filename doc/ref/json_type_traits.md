### jsoncons::json_type_traits

```c++
#include <jsoncons/json_type_traits.hpp>
```

<br>

`json_type_traits` defines a compile time template based interface for conversion between a `basic_json` value
and a value of some other type.

The traits class that `json_type_traits` implementations must specialize is as follows:

```c++
template <class Json, class T, class Enable=void>
struct json_type_traits
{
    using allocator_type = typename Json::allocator_type;

    static constexpr bool is(const Json&);

    static T as(const Json&);

    static Json to_json(const T&);

    static Json to_json(const T&, const allocator_type& = allocator_type());
};
```

jsoncons includes specializiations for most types in the standard library. 
And it includes convenience macros that make specializing `json_type_traits` for your own types easier.

[jsoncons Specializations](json_type_traits/jsoncons-specializations.md)

[Convenience Macros](json_type_traits/convenience-macros.md)

[Custom Specializations](json_type_traits/custom-specializations.md)


