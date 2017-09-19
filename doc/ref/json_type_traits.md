### jsoncons::json_type_traits

`json_type_traits` defines a compile time template based interface that `json` objects use to access and modify values.

#### Header
```c++
#include <jsoncons/json_type_traits.hpp>

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    typedef typename Json::allocator_type allocator_type;

    static const bool is_compatible = false;

    static bool is(const Json&)
    {
        return false;
    }

    static T as(const Json& rhs);

    static Json to_json(T val);

    static Json to_json(T val, allocator_type allocator);
};
```

### Specializations

`T`|`j.is<T>()`|`j.as<T>()`|j is assignable from `T`
--------|-----------|--------------|---
`Json`|`true`|self|<em>&#x2713;</em>
`Json::object`|`true` if `j.is_object()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`Json::array`|`true` if `j.is_array()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`bool`|`true` if `j.is_bool()`, otherwise `false`|as `bool`|<em>&#x2713;</em>
`null_type`|`true` if `j.is_null()`, otherwise `false`|`null_type()` value if j.is_null(), otherwise throws|<em>&#x2713;</em>
`const char_type*`|`true` if string, otherwise `false`|as `const char_type*`|<em>&#x2713;</em>
`char_type*`|`true` if `j.is_string()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`integral types`|`true` if `j.is_integer()` or `j.is_uinteger()` and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`floating point types`|`true` if j.is_double() and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`string`|`true` if j.is_string(), otherwise `false`|as string|<em>&#x2713;</em>
STL sequence container (other than string) e.g. std::vector|`true` if array and each value is assignable to a `Json` value, otherwise `false`|if array and each value is convertible to `value_type`, as container, otherwise throws|<em>&#x2713;</em>
STL associative container e.g. std::map|`true` if object and each `mapped_type` is assignable to `Json`, otherwise `false`|if object and each member value is convertible to `mapped_type`, as container|<em>&#x2713;</em>
`std::tuple`|`true` if `j.is_array()` and each array element is assignable to the corresponding `tuple` element, otherwise false|tuple with array elements converted to tuple elements|<em>&#x2713;</em>
`std::pair`|`true` if `j.is_array()` and `j.size()==2` and each array element is assignable to the corresponding pair element, otherwise false|pair with array elements converted to pair elements|<em>&#x2713;</em>

### Examples

#### Convert from and to standard library sequence containers

```c++
    std::vector<int> v{1, 2, 3, 4};
    json j(v);
    std::cout << "(1) "<< j << std::endl;
    std::deque<int> d = j.as<std::deque<int>>();
```
Output:
```
(1) [1,2,3,4]
```

#### Convert from and to standard library associative containers

```c++
    std::map<std::string,int> m{{"one",1},{"two",2},{"three",3}};
    json j(m);
    std::cout << "(1) " << j << std::endl;
    std::unordered_map<std::string,int> um = j.as<std::unordered_map<std::string,int>>();
```
Output:
```
(1) {"one":1,"three":3,"two":2}
```

#### Convert from and to std::tuple

```c++
    auto t = std::make_tuple(false,1,"foo");
    json j(t);
    std::cout << "(1) "<< j << std::endl;
    auto t2 = j.as<std::tuple<bool,int,std::string>>();
```
Output:
```
(1) [false,1,"foo"]
```

### Extensibility

You can interact with a new type using `is<T>`, `as<T>`, construction and assignment by extending `json_type_traits` in the `jsoncons` namespace.

For example, you can access and modify `json` values with `boost::gregorian` dates.

```c++
#include <jsoncons/json.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
        static const bool is_assignable = true;

        static bool is(const Json& val) noexcept
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
            try
            {
                boost::gregorian::from_simple_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static boost::gregorian::date as(const Json& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static Json to_json(boost::gregorian::date val)
        {
            return Json(to_iso_extended_string(val));
        }
    };
}
```
```c++
namespace my_ns
{
    using jsoncons::json;
    using boost::gregorian::date;

    json deal = json::parse(R"(
    {
        "Maturity":"2014-10-14",
        "ObservationDates": ["2014-02-14","2014-02-21"]
    }
    )");

    deal["ObservationDates"].push_back(date(2014,2,28));    

    date maturity = deal["Maturity"].as<date>();
    std::cout << "Maturity: " << maturity << std::endl << std::endl;

    std::cout << "Observation dates: " << std::endl << std::endl;

    for (auto observation_date: deal["ObservationDates"].array_range())
    {
        std::cout << observation_date << std::endl;
    }
    std::cout << std::endl;
}
```
Output:
```
Maturity: 2014-Oct-14

Observation dates:

2014-Feb-14
2014-Feb-21
2014-Feb-28
```
