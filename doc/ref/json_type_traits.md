### jsoncons::json_type_traits

`json_type_traits` defines a compile time template based interface for accessing and modifying `basic_json` values.

#### Header
```c++
#include <jsoncons/json_type_traits.hpp>
```

The default definition provided by the `jsoncons` library is

```c++
template <class Json, class T, class Enable=void>
struct json_type_traits
{
    typedef typename Json::allocator_type allocator_type;

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
struct is_json_type_traits_declared : public false_type {};
```
which inherits from [false_type](http://www.cplusplus.com/reference/type_traits/false_type/).
This traits class may be specialized for a user-defined type with a [true_type](http://www.cplusplus.com/reference/type_traits/true_type/) value to
inform the `jsoncons` library that the type is already specialized.  

`JSONCONS_MEMBER_TRAITS_DECL` is a macro that simplifies the creation of the necessary boilerplate
for your own types. If used, it must be placed outside any namespace blocks.

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
`integral types`|`true` if `j.is_int64()` or `j.is_uint64()` and value is in range, otherwise `false`|j numeric value cast to `T`|<em>&#x2713;</em>
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
    std::cout << j << std::endl;
    std::unordered_map<std::string,int> um = j.as<std::unordered_map<std::string,int>>();
```
Output:
```
{"one":1,"three":3,"two":2}
```

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

#### Extend json_type_traits to support `boost::gregorian` dates.

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

#### Specialize json_type_traits to support a book class.

```c++
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <jsoncons/json.hpp>

struct book
{
    std::string author;
    std::string title;
    double price;
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, book>
    {
        static bool is(const Json& j) noexcept
        {
            return j.is_object() &&
                   j.contains("author") && 
                   j.contains("title") && 
                   j.contains("price");
        }
        static book as(const Json& j)
        {
            book val;
            val.author = j["author"].template as<std::string>();
            val.title = j["title"].template as<std::string>();
            val.price = j["price"].template as<double>();
            return val;
        }
        static Json to_json(const book& val)
        {
            Json j;
            j["author"] = val.author;
            j["title"] = val.title;
            j["price"] = val.price;
            return j;
        }
    };
} // jsoncons

using jsoncons::json;

int main()
{
    book book1{"Haruki Murakami", "Kafka on the Shore", 25.17};

    json j = book1;

    std::cout << "(1) " << std::boolalpha << j.is<book>() << "\n\n";

    std::cout << "(2) " << pretty_print(j) << "\n\n";

    book temp = j.as<book>();
    std::cout << "(3) " << temp.author << "," 
                        << temp.title << "," 
                        << temp.price << "\n\n";

    book book2{"Charles Bukowski", "Women: A Novel", 12.0};

    std::vector<book> book_array{book1, book2};

    json ja = book_array;

    std::cout << "(4) " << std::boolalpha 
                        << ja.is<std::vector<book>>() << "\n\n";

    std::cout << "(5)\n" << pretty_print(ja) << "\n\n";

    auto book_list = ja.as<std::list<book>>();

    std::cout << "(6)" << std::endl;
    for (auto b : book_list)
    {
        std::cout << b.author << ", " 
                  << b.title << ", " 
                  << b.price << std::endl;
    }
}
```
Output:
```
(1) true

(2) {
    "author": "Haruki Murakami",
    "price": 25.17,
    "title": "Kafka on the Shore"
}

(3) Haruki Murakami,Kafka on the Shore,25

(4) true

(5)
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 12.0,
        "title": "Women: A Novel"
    }
]

(6)
Haruki Murakami, Kafka on the Shore, 25
Charles Bukowski, Women: A Novel, 12
{"1":2,"3":4}
```

#### Using JSONCONS_MEMBER_TRAITS_DECL to generate the json_type_traits 

`JSONCONS_MEMBER_TRAITS_DECL` is a macro that can be used to generate the `json_type_traits` boilerplate
for your own types.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

    struct reputon
    {
        std::string rater;
        std::string assertion;
        std::string rated;
        double rating;

        friend bool operator==(const reputon& lhs, const reputon& rhs)
        {
            return lhs.rater == rhs.rater && lhs.assertion == rhs.assertion && 
                   lhs.rated == rhs.rated && lhs.rating == rhs.rating;
        }

        friend bool operator!=(const reputon& lhs, const reputon& rhs)
        {
            return !(lhs == rhs);
        };
    };

    class reputation_object
    {
        std::string application;
        std::vector<reputon> reputons;

        // Make json_type_traits specializations friends to give accesses to private members
        JSONCONS_TYPE_TRAITS_FRIEND;
    public:
        reputation_object()
        {
        }
        reputation_object(const std::string& application, const std::vector<reputon>& reputons)
            : application(application), reputons(reputons)
        {}

        friend bool operator==(const reputation_object& lhs, const reputation_object& rhs)
        {
            return (lhs.application == rhs.application) && (lhs.reputons == rhs.reputons);
        }

        friend bool operator!=(const reputation_object& lhs, const reputation_object& rhs)
        {
            return !(lhs == rhs);
        };
    };


} // namespace ns

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_MEMBER_TRAITS_DECL(ns::reputon, rater, assertion, rated, rating)
JSONCONS_MEMBER_TRAITS_DECL(ns::reputation_object, application, reputons)

int main()
{
    ns::reputation_object val("hiking", { ns::reputon{"HikingAsylum.example.com","strong-hiker","Marilyn C",0.90} });

    std::string s;
    encode_json(val, s, indenting::indent);
    std::cout << s << "\n";

    auto val2 = decode_json<ns::reputation_object>(s);

    assert(val2 == val);
}
```
Output:
```
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "strong-hiker",
            "rated": "Marilyn C",
            "rater": "HikingAsylum.example.com",
            "rating": 0.9
        }
    ]
}
```

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
struct json_type_traits<Json, own_vector> {
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
		for(size_t i=0;i<val.size();i+=2){
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
    json j = json::object{ {"1",2},{"3",4} };
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

