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

### Examples

#### Uses allocator construction example

```cpp
#include <jsoncons/json.hpp>
#include <../examples/src/common/mock_stateful_allocator.hpp>
#include <scoped_allocator>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

namespace ns {

template <typename Alloc>
struct book
{
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    string_type author;
    string_type title;
    double price{0};

    book(const Alloc& alloc)
        : author(alloc), title(alloc)
    {
    }

    book(book&& other, const Alloc& alloc)
        : author(std::move(other.author), alloc), title(std::move(other.title), alloc)
    {
    }
};

} // namespace ns

namespace jsoncons {
namespace reflect {

template <typename Json, typename Alloc>
struct json_conv_traits<Json, ns::book<Alloc>>
{
    using value_type = ns::book<Alloc>;

    static bool is(const Json& j) noexcept
    {
        return j.is_object() && j.contains("author") &&
            j.contains("title") && j.contains("price");
    }
    template <typename Alloc, typename TempAlloc>
    static conversion_result<value_type> try_as(const allocator_set<Alloc,TempAlloc>& aset, const Json& j)
    {
        using result_type = conversion_result<value_type>;

        if (!j.is_object())
        {
            return result_type(jsoncons::unexpect, conv_errc::not_object, "ns::book");
        }

        auto val = jsoncons::make_obj_using_allocator<value_type>(aset.get_allocator());

        {
            auto it = j.find("author");
            if (it == j.object_range().end())
            {
                return result_type(jsoncons::unexpect, conv_errc::missing_required_member, "author");
            }
            auto r = it->value().template try_as<typename value_type::string_type>(aset);
            if (!r)
            {
                return result_type(unexpect, r.error().code());
            }
            val.author = std::move(*r);
        }
        {
            auto it = j.find("title");
            if (it == j.object_range().end())
            {
                return result_type(jsoncons::unexpect, conv_errc::missing_required_member, "title");
            }
            auto r = it->value().template try_as<typename value_type::string_type>(aset);
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error().code());
            }
            val.title = std::move(*r);
        }
        {
            auto it = j.find("price");
            if (it == j.object_range().end())
            {
                return result_type(jsoncons::unexpect, conv_errc::missing_required_member, "price");
            }
            auto r = it->value().template try_as<double>(aset);
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error().code(), "price");
            }
            val.price = *r;
        }

        return result_type(std::move(val));
    }

    template <typename Alloc, typename TempAlloc>
    static Json to_json(const allocator_set<Alloc, TempAlloc>& aset, const value_type& val)
    {
        auto j = jsoncons::make_obj_using_allocator<Json>(aset.get_allocator(), json_object_arg);
        j.try_emplace("author", val.author);
        j.try_emplace("title", val.title);
        j.try_emplace("price", val.price);
        return j;
    }
};

} // namespace reflect
} // namespace jsoncons

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using book_type = ns::book<cust_allocator<char>>;
using books_type = std::vector<book_type, cust_allocator<book_type>>;

int main()
{
    const std::string input = R"(
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

    cust_allocator<book_type> alloc(1);
    auto aset = jsoncons::make_alloc_set<cust_allocator<book_type>>(alloc);
    auto rin = jsoncons::try_decode_json<books_type>(aset, input);
    assert(rin);

    std::string output;
    auto rout = jsoncons::try_encode_json_pretty(aset, *rin, output);
    assert(rout);
    std::cout << output << "\n";
}
```
Output:
```
[
    {
        "author": "Haruki Murakami",
        "title": "Kafka on the Shore",
        "price": 0.0
    },
    {
        "author": "Charles Bukowski",
        "title": "Pulp",
        "price": 0.0
    }
]
```

To save typing and enhance readability, you can use the convenience macro `JSONCONS_ALL_MEMBER_TRAITS` 
to generate the traits classes,

```
JSONCONS_ALL_MEMBER_TRAITS(ns::book, author, title, price)
```


