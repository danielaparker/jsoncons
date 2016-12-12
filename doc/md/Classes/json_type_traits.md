```c++
jsoncons::json

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    typedef typename Json::char_type char_type;
    typedef typename Json::object object;
    typedef typename Json::array array;

    static const bool is_assignable = false;

    static bool is(const Json&)
    {
        return false;
    }

    static T as(const Json& rhs);

    static Json to_json(const T& rhs);
};
```

## Specializations
`T`|`is<T>`|`as<T>`|Assignable from `T`
--------|-----------|--------------|---
`Json`|`true`|self|<em>&#x2713;</em>
`object`|`true` if object, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`array`|`true` if array, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`bool`|`true` if boolean, otherwise `false`|as `bool`|<em>&#x2713;</em>
`null_type`|`true` if null, otherwise `false`|null value if null, otherwise throws|<em>&#x2713;</em>
`const char_type*`|`true` if string, otherwise `false`|as `const char_type*`|<em>&#x2713;</em>
`char_type*`|`true` if string, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`char`|`true` if integer and value in range, otherwise `false`|integer cast to `char`|<em>&#x2713;</em>
`unsigned char`|`true` if integer and value in range, otherwise `false`|integer cast to `unsigned char`|<em>&#x2713;</em>
`signed char`|`true` if integer and value in range, otherwise `false`|integer cast to `signed char`|<em>&#x2713;</em>
`wchar_t`|`true` if integer and value in range, otherwise `false`|integer cast to `wchar_t`|<em>&#x2713;</em>
`short`|`true` if integer and value in range, otherwise `false`|integer cast to `short`|<em>&#x2713;</em>
`unsigned short`|`true` if integer and value in range, otherwise `false`|integer cast to `unsigned short`|<em>&#x2713;</em>
`int`|`true` if integer and value in range, otherwise `false`|integer cast to `int`|<em>&#x2713;</em>
`unsigned int`|`true` if integer and value in range, otherwise `false`|integer cast to `unsigned int`|<em>&#x2713;</em>
`long`|`true` if integer and value in range, otherwise `false`|integer cast to `long`|<em>&#x2713;</em>
`unsigned long`|`true` if integer and value in range, otherwise `false`|integer cast to `unsigned long`|<em>&#x2713;</em>
`long  long`|`true` if integer and value in range, otherwise `false`|integer cast to `long  long`|<em>&#x2713;</em>
`unsigned long  long`|`true` if integer and value in range, otherwise `false`|integer cast to `unsigned long  long`|<em>&#x2713;</em>
`float`|`true` if integer and value in range, otherwise `false`|integer cast to `float`|<em>&#x2713;</em>
`double`|`true` if integer and value in range, otherwise `false`|integer cast to `double`|<em>&#x2713;</em>
`string`|`true` if string, otherwise `false`|as string|<em>&#x2713;</em>
STL sequence container (other than string) e.g. std::vector|`true` if array and each value is assignable to a `Json` value, otherwise `false`|if array and each value is convertible to `value_type`, as container, otherwise throws|<em>&#x2713;</em>
STL associative container e.g. std::map|`true` if object and each `mapped_type` is assignable to `Json`, otherwise `false`|if object and each member value is convertible to `mapped_type`, as container|<em>&#x2713;</em>

## Extensibility

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
            return Json::make_string(to_iso_extended_string(val));
        }

        static Json to_json(boost::gregorian::date val)
        {
            return Json::make_string(to_iso_extended_string(val));
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

    deal["ObservationDates"].add(date(2014,2,28));    

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
