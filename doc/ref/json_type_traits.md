### jsoncons::json_type_traits

```c++
#include <jsoncons/json_type_traits.hpp>
```

<br>

`json_type_traits` defines a compile time template based interface for accessing and modifying `basic_json` values.

The default definition provided by the `jsoncons` library is

```c++
template <class Json, class T, class Enable=void>
struct json_type_traits
{
    using allocator_type = typename Json::allocator_type;

    static constexpr bool is_compatible = false;

    static constexpr bool is(const Json&)
    {
        return false;
    }

    static T as(const Json&);

    static Json to_json(const T&, allocator_type = allocator_type());
};
```

You can interact with a new type using `is<T>`, `as<T>`, construction and assignment by specializing `json_type_traits` in the `jsoncons` namespace.

If you try to specialize `json_type_traits` for a type that is already
specialized in the jsoncons library, for example, a custom container 
type that satisfies the conditions for a sequence container, you 
may see a compile error "more than one partial specialization matches the template argument list".
For these situations `jsoncons` provides the traits class
```c++
template <class T>
struct is_json_type_traits_declared : public std::false_type {};
```
which inherits from [std::false_type](http://www.cplusplus.com/reference/type_traits/false_type/).
This traits class may be specialized for a user-defined type with member constant `value` equal `true`
to inform the `jsoncons` library that the type is already specialized.  

### jsoncons specializations

`T`|`j.is<T>()`|`j.as<T>()`|j is assignable from `T`
--------|-----------|--------------|---
`Json`|`true`|self|<em>&#x2713;</em>
`Json::object`|`true` if `j.is_object()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`Json::array`|`true` if `j.is_array()`, otherwise `false`|Compile-time error|<em>&#x2713;</em>
`bool`|`true` if `j.is_bool()`, otherwise `false`|as `bool`|<em>&#x2713;</em>
`null_type`|`true` if `j.is_null()`, otherwise `false`|`null_type()` value if j.is_null(), otherwise throws|<em>&#x2713;</em>
`integral types`|`true` if `j.is_int64()` or `j.is_uint64()` and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`floating point types`|`true` if j.is_double() and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
`std::basic_string<CharT>`<sup>1</sup>|`true` if `j.is<std::basic_string<CharT>>()`, otherwise `false`|j.as<std::basic_string<CharT>>|<em>&#x2713;</em>
`jsoncons::basic_string_view<CharT>`<sup>1</sup><sup>,2</sup>|`true` if `j.is<jsoncons::basic_string_view<CharT>>()`, otherwise `false`|j.as<std::basic_string_view<CharT>>|<em>&#x2713;</em>
STL sequence container (other than string) e.g. std::vector|`true` if array and each value is assignable to a `Json` value, otherwise `false`|if array and each value is convertible to `value_type`, as container, otherwise throws|<em>&#x2713;</em>
STL associative container e.g. `std::map<K,U>`|`true` if object and each `mapped_type` is assignable to `Json`, otherwise `false`|if object and each member value is convertible to `mapped_type`, as container|<em>&#x2713;</em>
`std::tuple<Args...>`|`true` if `j.is_array()` and each array element is assignable to the corresponding `tuple` element, otherwise false|tuple with array elements converted to tuple elements|<em>&#x2713;</em>
`std::pair<U,V>`|`true` if `j.is_array()` and `j.size()==2` and each array element is assignable to the corresponding pair element, otherwise false|pair with array elements converted to pair elements|<em>&#x2713;</em>
`std::shared_ptr<U>`<sup>3</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty shared_ptr if `j.is_null()`, otherwise `make_shared(j.as<U>())`|<em>&#x2713;</em>
`std::unique_ptr<U>`<sup>4</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty unique_ptr if `j.is_null()`, otherwise `make_unique(j.as<U>())`|<em>&#x2713;</em>
`jsoncons::optional<U>`<sup>5</sup>|`true` if `j.is_null()` or `j.is<U>()`|Empty `jsoncons::optional<U>` if `j.is_null()`, otherwise `jsoncons::optional<U>(j.as<U>())`|<em>&#x2713;</em>
`std::variant<Types...>`<sup>6</sup>|&nbsp;|<em>&#x2713;</em>
  
1. For `CharT` `char` or `wchar_t`.
2. `jsoncons::basic_string_view<CharT>` is aliased to [std::basic_string_view<CharT>](https://en.cppreference.com/w/cpp/utility/optional) if 
jsoncons detects the presence of C++17, or if `JSONCONS_HAS_STD_STRING_VIEW` is defined.  
3. Defined if `U` is not a polymorphic class, i.e., does not have any virtual functions.  
4. Defined if `U` is not a polymorphic class, i.e., does not have any virtual functions.   
5. `jsoncons::optional<U>` is aliased to [std::optional<U>](https://en.cppreference.com/w/cpp/utility/optional) if 
jsoncons detects the presence of C++17, or if `JSONCONS_HAS_STD_OPTIONAL` is defined.
6. Since v0.154.0  

### Convenience Macros

The `jsoncons` library provides a number of macros that can be used to generate the code to specialize `json_type_traits`
for a user-defined class.

Macro names include qualifiers `_ALL_` or `_N_` to indicate that the generated traits require all
members be present in the JSON, or only a specified number be present. For non-mandatory members,
empty values for `std::shared_ptr`, `std::unique_ptr` and `std::optional` are excluded altogether
when serializing. For mandatory members, empty values for `std::shared_ptr`, `std::unique_ptr` and `std::optional` 
become JSON null when serializing.

The qualifer `_TPL` indicates that the generated traits are for a template class with a specified number
of template parameters.

```c++
JSONCONS_N_MEMBER_TRAITS(class_name,num_mandatory,
                         member_name0,member_name1,...) // (1)

JSONCONS_ALL_MEMBER_TRAITS(class_name,
                           member_name0,member_name1,...) // (2)

JSONCONS_TPL_N_MEMBER_TRAITS(num_template_params,
                             class_name,num_mandatory,
                             member_name0,member_name1,...) // (3)  

JSONCONS_TPL_ALL_MEMBER_TRAITS(num_template_params,
                               class_name,
                               member_name0,member_name1,...) // (4)

JSONCONS_N_MEMBER_NAME_TRAITS(class_name,num_mandatory,
                              (member_name0,serialized_name0),
                              (member_name1,serialized_name1)...) // (5)

JSONCONS_ALL_MEMBER_NAME_TRAITS(class_name,
                                (member_name0,serialized_name0),
                                (member_name1,serialized_name1)...) // (6)

JSONCONS_TPL_N_MEMBER_NAME_TRAITS(num_template_params,
                                  class_name,num_mandatory,
                                  (member_name0,serialized_name0),
                                  (member_name1,serialized_name1)...) // (7)

JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(num_template_params,
                                    class_name,
                                    (member_name0,serialized_name0),
                                    (member_name1,serialized_name1)...) // (8)

JSONCONS_ENUM_TRAITS(enum_name,enumerator0,enumerator1,...) // (9)

JSONCONS_ENUM_NAME_TRAITS(enum_name,
                           (enumerator0,serialized_name0),
                           (enumerator1,serialized_name1)...) // (10)

JSONCONS_N_CTOR_GETTER_TRAITS(class_name,num_mandatory,
                              getter_name0,
                              getter_name1,...) // (11)

JSONCONS_ALL_CTOR_GETTER_TRAITS(class_name,
                                getter_name0,getter_name1,...) // (12)

JSONCONS_TPL_N_CTOR_GETTER_TRAITS(num_template_params,
                                  class_name,num_mandatory,
                                  getter_name0,getter_name1,...) // (13)

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(num_template_params,
                                    class_name,
                                    getter_name0,getter_name1,...) // (14)

JSONCONS_N_CTOR_GETTER_NAME_TRAITS(class_name,num_mandatory,
                                   (getter_name0,serialized_name0),
                                   (getter_name1,serialized_name1)...) // (15)

JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS(class_name,
                                     (getter_name0,serialized_name0),
                                     (getter_name1,serialized_name1)...) // (16)

JSONCONS_TPL_N_CTOR_GETTER_NAME_TRAITS(num_template_params,
                                       class_name,num_mandatory,
                                       (getter_name0,serialized_name0),
                                       (getter_name1,serialized_name1)...) // (17)

JSONCONS_TPL_ALL_CTOR_GETTER_NAME_TRAITS(num_template_params,
                                         class_name,
                                         (getter_name0,serialized_name0),
                                         (getter_name1,serialized_name1)...) // (18)

JSONCONS_N_GETTER_SETTER_TRAITS(class_name,get_prefix,set_prefix,num_mandatory,
                                field_name0,field_name1,...) // (19)

JSONCONS_ALL_GETTER_SETTER_TRAITS(class_name,get_prefix,set_prefix,
                                  field_name0,field_name1,...) // (20)

JSONCONS_TPL_N_GETTER_SETTER_TRAITS(num_template_params,
                                    class_name,get_prefix,set_prefix,num_mandatory,
                                    field_name0,field_name1,...) // (21)  

JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS(num_template_params,
                                      class_name,get_prefix,set_prefix,
                                      field_name0,field_name1,...) // (22)

JSONCONS_N_GETTER_SETTER_NAME_TRAITS(class_name,num_mandatory,
                                     (getter_name0,setter_name0,serialized_name0),
                                     (getter_name1,setter_name1,serialized_name1)...) // (23)

JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(class_name,
                                       (getter_name0,setter_name0,serialized_name0),
                                       (getter_name1,setter_name1,serialized_name1)...) // (24)

JSONCONS_TPL_N_GETTER_SETTER_NAME_TRAITS(num_template_params,
                                         class_name,num_mandatory,
                                         (getter_name0,setter_name0,serialized_name0),
                                         (getter_name1,setter_name1,serialized_name1)...) // (25)

JSONCONS_TPL_ALL_GETTER_SETTER_NAME_TRAITS(num_template_params,
                                           class_name,
                                           (getter_name0,setter_name0,serialized_name0),
                                           (getter_name1,setter_name1,serialized_name1)...) // (26)

JSONCONS_POLYMORPHIC_TRAITS(base_class_name,derived_class_name0,derived_class_name1,...) // (27)
```

(1)-(4) generate the code to specialize `json_type_traits` for a class from member data. 
The serialized names are the stringified member names. 
When decoding to a C++ data structure, 
(1) and (3) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (2) and (4)
require that all member names be present in the JSON. The class must have a default constructor.
If the member data or default constructor are private, the macro `JSONCONS_TYPE_TRAITS_FRIEND`
will make them accessible to `json_type_traits`, used so
 
```c++
class MyClass
{
    JSONCONS_TYPE_TRAITS_FRIEND
...
};
```

(3)-(4) generate the code to specialize `json_type_traits` for a class template from member data. 

(5)-(8) generate the code to specialize `json_type_traits` for a class from member data.
The serialized names are the provided names. The sequence of `(member_nameN,serialized_nameN)`
pairs declares the member name and provided name for each of the class members
that are part of the sequence.
When decoding to a C++ data structure, 
(5) and (7) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (6) and (8) 
require that all member names be present in the JSON. The class must have a default constructor.
If the member data or default constructor are private, the macro `JSONCONS_TYPE_TRAITS_FRIEND`
will make them accessible to `json_type_traits`.
(7)-(8) generate the code to specialize `json_type_traits` for a class template from member data. 

(9) generates the code to specialize `json_type_traits` for an enumerated type from its enumerators.
The serialized name is the stringified enumerator name. 

(10) generates the code to specialize `json_type_traits` for an enumerated type from its enumerators.
The serialized name is the provided name. The sequence of `(enumeratorN,serialized_nameN)`
pairs declares the named constant and provided name for each of the enumerators
that are part of the sequence.

(11)-(14) generate the code to specialize `json_type_traits` for a class from a constructor and get functions. 
The serialized names are the stringified field names. 
When decoding to a C++ data structure, 
(11) and (13) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (12) and (14) 
require that all member names be present in the JSON. The class must have a constructor such that the return types 
of the get functions are convertible to its parameters, taken in order. 
(13)-(14) generate the code to specialize `json_type_traits` for a class template from a constructor and get functions.  

(15)-(18) generate the code to specialize `json_type_traits` for a class from a constructor and get functions.
The serialized names are the provided names. The sequence of `(getter_nameN,serialized_nameN)`
pairs declares the get function and provided name for each of the class members
that are part of the sequence. 
When decoding to a C++ data structure, 
(15) and (17) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (16) and (18) 
require that all member names be present in the JSON. The class must have a constructor such that the return types 
of the get functions are convertible to its parameters, taken in order. 
(17)-(18) generate the code to specialize `json_type_traits` for a class template from a constructor and get functions.  

(19)-(22) generate the code to specialize `json_type_traits` for a class from get and set functions.
The serialized names are the stringified field names. The get and set function names are
formed from the concatenation of `get_prefix` and `set_prefix` with field name.
(19) and (21) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (20) and (22) 
require that all member names be present in the JSON. (21)-(22) generate the code to specialize `json_type_traits` 
for a class template from get and set functions.

(23)-(26) generate the code to specialize `json_type_traits` for a class from get and set functions.
The serialized names are the provided names. The sequence of `(getter_nameN,setter_nameN,serialized_nameN)`
triples declares the get and set functions and provided name for each of the class members
that are part of the sequence. When decoding to a C++ data structure, 
(23) and (25) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (24) and (26) 
require that all member names be present in the JSON. The class must have a default constructor. 
(25)-(26) generate the code to specialize `json_type_traits` for a class template from get and set functions.

(27) generates the code to specialize `json_type_traits` for `std::shared_ptr<base_class>` and `std::unique_ptr<base_class>`.
Each derived class must have a `json_type_traits<Json,derived_class_name>` specialization.
The type selection strategy is based on `json_type_traits<Json,derived_class_name>::is(const Json& j)`.
In the case that `json_type_traits<Json,derived_class_name>` has been generated by one of the
conveniences macros (1)-(26), the type selection strategy is based on the presence of members
in the derived classes.

#### Parameters

`class_name` - the name of a class or struct  
`num_mandatory` - the number of mandatory class data members or accessors  
`enum_name` - the name of an enum type or enum class type  
`num_template_params` - for a class template, the number of template parameters  
`member_nameN` - the name of a class data member. Class data members are normally modifiable, but may be `const` or
`static const`. Data members that are `const` or `static const` are one-way serialized.  
`getter_nameN` - the getter for a class data member  
`(enumeratorN,serialized_nameN)` - an enumerator and corresponding serialized name  
`(getter_nameN,serialized_nameN)` - the getter for a class data member and corresponding serialized name  
`field_nameN` - the base name of a class getter or setter with prefix `get` or `set` stripped out.  
`(getter_nameN,setter_nameN,serialized_nameN)` - the getter and setter for a class data member, and corresponding serialized name  
`base_class_name` - the name of a base class  
`derived_class_nameN` - a class that is derived from the base class, and that has a `json_type_traits<Json,derived_class_nameN>` specialization.  

These macro declarations must be placed at global scope, outside any namespace blocks, and `class_name`, 
`base_class_name` and `derived_class_nameN` must be a fully namespace qualified names.

All of the `json_type_traits` specializations for type `T` generated by the convenience macros include a specialization of
`is_json_type_traits_declared<T>` with member constant `value` equal `true`.

### Examples

[Convert from and to standard library sequence containers](#A1)  
[Convert from and to standard library associative containers](#A2)  
[Convert from and to std::map with integer key](#A3)  
[Convert from and to std::tuple](#A4)  
[Extend json_type_traits to support `boost::gregorian` dates.](#A5)  
[Specialize json_type_traits to support a book class.](#A6)  
[Using JSONCONS_ALL_CTOR_GETTER_TRAITS to generate the json_type_traits](#A7)  
[Example with std::shared_ptr, std::unique_ptr and std::optional](#A8)  
[Serialize a polymorphic type based on the presence of members](#A9)  
[Ensuring type selection is possible](#A10)  
[Specialize json_type_traits for a container type that the jsoncons library also supports](#A11)  
[Convert JSON to/from boost matrix](#A12)

<div id="A1"/> 

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

<div id="A2"/> 

#### Convert from and to standard library associative containers

```c++
std::map<std::string,int> m{{"one",1},{"two",2},{"three",3}};
json j(m);
std::cout << j << std::endl;
std::unordered_map<std::string,int> um = j.as<std::unordered_map<std::string,int>>();
```
Output:
```
{"one":1,"three":3,"two":2}
```

<div id="A3"/> 

#### Convert from and to std::map with integer key

```c++
std::map<short, std::string> m{ {1,"foo",},{2,"baz"} };

json j{m};

std::cout << "(1)\n";
std::cout << pretty_print(j) << "\n\n";

auto other = j.as<std::map<uint64_t, std::string>>();

std::cout << "(2)\n";
for (const auto& item : other)
{
    std::cout << item.first << " | " << item.second << "\n";
}
std::cout << "\n\n";
```
Output:

```c++
(1)
{
    "1": "foo",
    "2": "baz"
}

(2)
1 | foo
2 | baz
```

<div id="A4"/> 

#### Convert from and to std::tuple

```c++
    auto t = std::make_tuple(false,1,"foo");
    json j(t);
    std::cout << j << std::endl;
    auto t2 = j.as<std::tuple<bool,int,std::string>>();
```
Output:
```
[false,1,"foo"]
```

<div id="A5"/> 

#### Extend json_type_traits to support `boost::gregorian` dates.

```c++
#include <jsoncons/json.hpp>
#include "boost/datetime/gregorian/gregorian.hpp"

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
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
namespace ns
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

<div id="A6"/> 

#### Specialize json_type_traits to support a book class.

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>
#include <string>

namespace ns {
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
} // namespace ns

namespace jsoncons {

    template<class Json>
    struct json_type_traits<Json, ns::book>
    {
        using allocator_type = typename Json::allocator_type;

        static bool is(const Json& j) noexcept
        {
            return j.is_object() && j.contains("author") && 
                   j.contains("title") && j.contains("price");
        }
        static ns::book as(const Json& j)
        {
            ns::book val;
            val.author = j.at("author").template as<std::string>();
            val.title = j.at("title").template as<std::string>();
            val.price = j.at("price").template as<double>();
            return val;
        }
        static Json to_json(const ns::book& val, 
                            allocator_type alloc=allocator_type())
        {
            Json j(alloc);
            j.try_emplace("author", val.author);
            j.try_emplace("title", val.title);
            j.try_emplace("price", val.price);
            return j;
        }
    };
} // namespace jsoncons
```

To save typing and enhance readability, the jsoncons library defines macros, 
so you could also write

```c++
JSONCONS_N_MEMBER_TRAITS(ns::book, author, title, price)
```

which expands to the code above.

```c++
using namespace jsoncons; // for convenience

int main()
{
    const std::string s = R"(
    [
        {
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "author" : "Charles Bukowski",
            "title" : "Pulp",
            "price" : 22.48
        }
    ]
    )";

    std::vector<ns::book> book_list = decode_json<std::vector<ns::book>>(s);

    std::cout << "(1)\n";
    for (const auto& item : book_list)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(book_list, std::cout, indenting::indent);
    std::cout << "\n\n";
}
```
Output:
```
(1)
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Pulp, 22.48

(2)
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 22.48,
        "title": "Pulp"
    }
]
```

<div id="A7"/> 

#### Using JSONCONS_ALL_CTOR_GETTER_TRAITS to generate the json_type_traits 

The macro `JSONCONS_ALL_CTOR_GETTER_TRAITS` will generate the `json_type_traits` boilerplate
for your own types from a constructor and getter functions.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

namespace ns {
    enum class hiking_experience {beginner,intermediate,advanced};

    class hiking_reputon
    {
        std::string rater_;
        hiking_experience assertion_;
        std::string rated_;
        double rating_;
    public:
        hiking_reputon(const std::string& rater,
                       hiking_experience assertion,
                       const std::string& rated,
                       double rating)
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating)
        {
        }

        const std::string& rater() const {return rater_;}
        hiking_experience assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}

        friend bool operator==(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return lhs.rater_ == rhs.rater_ && lhs.assertion_ == rhs.assertion_ && 
                   lhs.rated_ == rhs.rated_ && lhs.rating_ == rhs.rating_;
        }

        friend bool operator!=(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return !(lhs == rhs);
        };
    };

    class hiking_reputation
    {
        std::string application_;
        std::vector<hiking_reputon> reputons_;
    public:
        hiking_reputation(const std::string& application, 
                          const std::vector<hiking_reputon>& reputons)
            : application_(application), 
              reputons_(reputons)
        {}

        const std::string& application() const { return application_;}
        const std::vector<hiking_reputon>& reputons() const { return reputons_;}

        friend bool operator==(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return (lhs.application_ == rhs.application_) && (lhs.reputons_ == rhs.reputons_);
        }

        friend bool operator!=(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return !(lhs == rhs);
        };
    };

} // namespace ns

using namespace jsoncons; // for convenience

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_ENUM_TRAITS(ns::hiking_experience, beginner, intermediate, advanced)
JSONCONS_ALL_CTOR_GETTER_TRAITS(ns::hiking_reputon, rater, assertion, rated, rating)
JSONCONS_ALL_CTOR_GETTER_TRAITS(ns::hiking_reputation, application, reputons)

int main()
{
    ns::hiking_reputation val("hiking", { ns::hiking_reputon{"HikingAsylum",ns::hiking_experience::advanced,"Marilyn C",0.90} });

    std::string s;
    encode_json(val, s, indenting::indent);
    std::cout << s << "\n";

    auto val2 = decode_json<ns::hiking_reputation>(s);

    assert(val2 == val);
}
```
Output:
```
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        }
    ]
}
```

<div id="A8"/> 

#### Example with std::shared_ptr, std::unique_ptr and std::optional

This example assumes C++17 language support for `std::optional`.
Lacking that, you can use `jsoncons::optional`.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

namespace ns {
    struct smart_pointer_and_optional_test
    {
        std::shared_ptr<std::string> field1;
        std::unique_ptr<std::string> field2;
        std::optional<std::string> field3;
        std::shared_ptr<std::string> field4;
        std::unique_ptr<std::string> field5;
        std::optional<std::string> field6;
        std::shared_ptr<std::string> field7;
        std::unique_ptr<std::string> field8;
        std::optional<std::string> field9;
        std::shared_ptr<std::string> field10;
        std::unique_ptr<std::string> field11;
        std::optional<std::string> field12;
    };

} // namespace ns

// Declare the traits, first 6 members mandatory, last 6 non-mandatory
JSONCONS_N_MEMBER_TRAITS(ns::smart_pointer_and_optional_test,6,
                         field1,field2,field3,field4,field5,field6,
                         field7,field8,field9,field10,field11,field12)

using namespace jsoncons; // for convenience

int main()
{
    ns::smart_pointer_and_optional_test val;
    val.field1 = std::make_shared<std::string>("Field 1"); 
    val.field2 = jsoncons::make_unique<std::string>("Field 2"); 
    val.field3 = "Field 3";
    val.field4 = std::shared_ptr<std::string>(nullptr);
    val.field5 = std::unique_ptr<std::string>(nullptr);
    val.field6 = std::optional<std::string>();
    val.field7 = std::make_shared<std::string>("Field 7"); 
    val.field8 = jsoncons::make_unique<std::string>("Field 8"); 
    val.field9 = "Field 9";
    val.field10 = std::shared_ptr<std::string>(nullptr);
    val.field11 = std::unique_ptr<std::string>(nullptr);
    val.field12 = std::optional<std::string>();

    std::string buf;
    encode_json(val, buf, indenting::indent);
    std::cout << buf << "\n";

    auto other = decode_json<ns::smart_pointer_and_optional_test>(buf);

    assert(*other.field1 == *val.field1);
    assert(*other.field2 == *val.field2);
    assert(*other.field3 == *val.field3);
    assert(!other.field4);
    assert(!other.field5);
    assert(!other.field6);
    assert(*other.field7 == *val.field7);
    assert(*other.field8 == *val.field8);
    assert(*other.field9 == *val.field9);
    assert(!other.field10);
    assert(!other.field11);
    assert(!other.field12);
}
```
Output:
```
{
    "field1": "Field 1",
    "field2": "Field 2",
    "field3": "Field 3",
    "field4": null,
    "field5": null,
    "field6": null,
    "field7": "Field 7",
    "field8": "Field 8",
    "field9": "Field 9"
}
```

<div id="A9"/> 

#### Serialize a polymorphic type based on the presence of members

This example uses the convenience macro `JSONCONS_N_CTOR_GETTER_TRAITS`
to generate the `json_type_traits` boilerplate for the `HourlyEmployee` and `CommissionedEmployee` 
derived classes, and `JSONCONS_POLYMORPHIC_TRAITS` to generate the `json_type_traits` boilerplate
for `std::shared_ptr<Employee>` and `std::unique_ptr<Employee>`. The type selection strategy is based
on the presence of mandatory members, in particular, to the `firstName`, `lastName`, and `wage` members of an
`HourlyEmployee`, and to the `firstName`, `lastName`, `baseSalary`, and `commission` members of a `CommissionedEmployee`.
Non-mandatory members are not considered for the purpose of type selection.

```c++
#include <cassert>
#include <iostream>
#include <vector>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

class Employee
{
    std::string firstName_;
    std::string lastName_;
public:
    Employee(const std::string& firstName, const std::string& lastName)
        : firstName_(firstName), lastName_(lastName)
    {
    }
    virtual ~Employee() noexcept = default;

    virtual double calculatePay() const = 0;

    const std::string& firstName() const {return firstName_;}
    const std::string& lastName() const {return lastName_;}
};

class HourlyEmployee : public Employee
{
    double wage_;
    unsigned hours_;
public:
    HourlyEmployee(const std::string& firstName, const std::string& lastName, 
                   double wage, unsigned hours)
        : Employee(firstName, lastName), 
          wage_(wage), hours_(hours)
    {
    }

    double wage() const {return wage_;}

    unsigned hours() const {return hours_;}

    double calculatePay() const override
    {
        return wage_*hours_;
    }
};

class CommissionedEmployee : public Employee
{
    double baseSalary_;
    double commission_;
    unsigned sales_;
public:
    CommissionedEmployee(const std::string& firstName, const std::string& lastName, 
                         double baseSalary, double commission, unsigned sales)
        : Employee(firstName, lastName), 
          baseSalary_(baseSalary), commission_(commission), sales_(sales)
    {
    }

    double baseSalary() const
    {
        return baseSalary_;
    }

    double commission() const
    {
        return commission_;
    }

    unsigned sales() const
    {
        return sales_;
    }

    double calculatePay() const override
    {
        return baseSalary_ + commission_*sales_;
    }
};

} // ns

JSONCONS_N_CTOR_GETTER_TRAITS(ns::HourlyEmployee, 3, firstName, lastName, wage, hours)
JSONCONS_N_CTOR_GETTER_TRAITS(ns::CommissionedEmployee, 4, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

int main()
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
    )"; 

    auto v = decode_json<std::vector<std::unique_ptr<ns::Employee>>>(input);

    std::cout << "(1)\n";
    for (const auto& p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << ", " << p->calculatePay() << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(v, std::cout, indenting::indent);

    std::cout << "\n\n(3)\n";
    json j(v);
    std::cout << pretty_print(j) << "\n\n";
}
```
Output:
```
(1)
John Smith, 40000
Jane Doe, 30250

(2)
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]

(3)
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
```

<div id="A10"/>

#### Ensuring type selection is possible

When deserializing a polymorphic type, jsoncons needs to know how
to convert a json value to the proper derived class. In the Employee
example above, the type selection strategy is based
on the presence of members in the derived classes. If
derived classes cannot be distinguished in this way, 
you can introduce extra members. The convenience
macros `JSONCONS_N_MEMBER_TRAITS`, `JSONCONS_ALL_MEMBER_TRAITS`,
`JSONCONS_TPL_N_MEMBER_TRAITS`, `JSONCONS_TPL_ALL_MEMBER_TRAITS`,
`JSONCONS_N_MEMBER_NAME_TRAITS`, `JSONCONS_ALL_MEMBER_NAME_TRAITS`,
`JSONCONS_TPL_N_MEMBER_NAME_TRAITS`, and `JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS`
allow you to have `const` or `static const` data members that are serialized and that 
particpate in the type selection strategy during deserialization. 

```c++
namespace ns {

class Foo
{
public:
    virtual ~Foo() noexcept = default;
};

class Bar : public Foo
{
    static const bool bar = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

class Baz : public Foo 
{
    static const bool baz = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

} // ns

JSONCONS_N_MEMBER_TRAITS(ns::Bar,1,bar)
JSONCONS_N_MEMBER_TRAITS(ns::Baz,1,baz)
JSONCONS_POLYMORPHIC_TRAITS(ns::Foo, ns::Bar, ns::Baz)

int main()
{
    std::vector<std::unique_ptr<ns::Foo>> u;
    u.emplace_back(new ns::Bar());
    u.emplace_back(new ns::Baz());

    std::string buffer;
    encode_json(u, buffer);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v = decode_json<std::vector<std::unique_ptr<ns::Foo>>>(buffer);

    std::cout << "(2)\n";
    for (const auto& ptr : v)
    {
        if (dynamic_cast<ns::Bar*>(ptr.get()))
        {
            std::cout << "A bar\n";
        }
        else if (dynamic_cast<ns::Baz*>(ptr.get()))
        {
            std::cout << "A baz\n";
        } 
    }
}
```

Output:
```
(1)
[{"bar":true},{"baz":true}]

(2)
A bar
A baz
```

<div id="A10"/> 

#### Specialize json_type_traits for a container type that the jsoncons library also supports

```c++
#include <cassert>
#include <string>
#include <vector>
#include <jsoncons/json.hpp>

//own vector will always be of an even length 
struct own_vector : std::vector<int64_t> { using  std::vector<int64_t>::vector; };

namespace jsoncons {

template<class Json>
struct json_type_traits<Json, own_vector> 
{
    static bool is(const Json& j) noexcept
    { 
        return j.is_object() && j.size() % 2 == 0;
    }
    static own_vector as(const Json& j)
    {   
        own_vector v;
        for (auto& item : j.object_range())
        {
            std::string s(item.key());
            v.push_back(std::strtol(s.c_str(),nullptr,10));
            v.push_back(item.value().template as<int64_t>());
        }
        return v;
    }
    static Json to_json(const own_vector& val){
		Json j;
		for(std::size_t i=0;i<val.size();i+=2){
			j[std::to_string(val[i])] = val[i + 1];
		}
		return j;
	}
};

template <> 
struct is_json_type_traits_declared<own_vector> : public std::true_type 
{}; 
} // jsoncons

using jsoncons::json;

int main()
{
    json j(json::object_arg, {{"1",2},{"3",4}});
    assert(j.is<own_vector>());
    auto v = j.as<own_vector>();
    json j2 = v;

    std::cout << j2 << "\n";
}
```
Output:
```
{"1":2,"3":4}
```

<div id="A12"/>

#### Convert JSON to/from boost matrix

```c++
#include <jsoncons/json.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace jsoncons {

    template <class Json, class T>
    struct json_type_traits<Json,boost::numeric::ublas::matrix<T>>
    {
        static bool is(const Json& val) noexcept
        {
            if (!val.is_array())
            {
                return false;
            }
            if (val.size() > 0)
            {
                std::size_t n = val[0].size();
                for (const auto& a: val.array_range())
                {
                    if (!(a.is_array() && a.size() == n))
                    {
                        return false;
                    }
                    for (auto x: a.array_range())
                    {
                        if (!x.template is<T>())
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        static boost::numeric::ublas::matrix<T> as(const Json& val)
        {
            if (val.is_array() && val.size() > 0)
            {
                std::size_t m = val.size();
                std::size_t n = 0;
                for (const auto& a : val.array_range())
                {
                    if (a.size() > n)
                    {
                        n = a.size();
                    }
                }

                boost::numeric::ublas::matrix<T> A(m,n,T());
                for (std::size_t i = 0; i < m; ++i)
                {
                    const auto& a = val[i];
                    for (std::size_t j = 0; j < a.size(); ++j)
                    {
                        A(i,j) = a[j].template as<T>();
                    }
                }
                return A;
            }
            else
            {
                boost::numeric::ublas::matrix<T> A;
                return A;
            }
        }

        static Json to_json(const boost::numeric::ublas::matrix<T>& val)
        {
            Json a = Json::template make_array<2>(val.size1(), val.size2(), T());
            for (std::size_t i = 0; i < val.size1(); ++i)
            {
                for (std::size_t j = 0; j < val.size1(); ++j)
                {
                    a[i][j] = val(i,j);
                }
            }
            return a;
        }
    };
} // jsoncons

using namespace jsoncons;
using boost::numeric::ublas::matrix;

int main()
{
    matrix<double> A(2, 2);
    A(0, 0) = 1.1;
    A(0, 1) = 2.1;
    A(1, 0) = 3.1;
    A(1, 1) = 4.1;

    json a = A;

    std::cout << "(1) " << std::boolalpha << a.is<matrix<double>>() << "\n\n";

    std::cout << "(2) " << std::boolalpha << a.is<matrix<int>>() << "\n\n";

    std::cout << "(3) \n\n" << pretty_print(a) << "\n\n";

    matrix<double> B = a.as<matrix<double>>();

    std::cout << "(4) \n\n";
    for (std::size_t i = 0; i < B.size1(); ++i)
    {
        for (std::size_t j = 0; j < B.size2(); ++j)
        {
            if (j > 0)
            {
                std::cout << ",";
            }
            std::cout << B(i, j);
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}
```
Output:
```
(1) true

(2) false

(3)

[
    [1.1,2.1],
    [3.1,4.1]
]

(4)

1.1,2.1
3.1,4.1
``` 

