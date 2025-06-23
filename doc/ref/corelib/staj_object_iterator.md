### jsoncons::staj_object_iterator

```cpp
#include <jsoncons/staj_iterator.hpp>

template<
    typename Key,
    typename T,
    typename CharT=char
    > class staj_object_iterator
```

A `staj_object_iterator` is an [InputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) that
accesses the individual stream events from a [staj_cursor](staj_cursor.md) and, provided that when it is constructed
the current stream event has type `begin_object`, it returns the elements
of the JSON object as items of type `std::pair<string_type,T>`. If when it is constructed the current stream event
does not have type `begin_object`, it becomes equal to the default-constructed iterator.

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|`CharT`
`key_type`|`Key`
`value_type`|`std::pair<Key,T>`
`difference_type`|`std::ptrdiff_t`
`pointer`|`value_type*`
`reference`|`value_type&`
`iterator_category`|[std::input_iterator_tag](https://en.cppreference.com/w/cpp/iterator/iterator_tags)

#### Constructors

    staj_object_iterator() noexcept;                            (1)

    staj_object_iterator(basic_staj_cursor<char_type>& cursor); (2)

    staj_object_iterator(basic_staj_cursor<char_type>& cursor,
        std::error_code& ec);                                   (3)

    staj_object_iterator(const staj_object_iterator& iter);     (4)

(1) Constructs the end iterator

(2) Constructs a `staj_object_iterator` that refers to the first member of the object
    following the current stream event `begin_object`. If there is no such member,
    returns the end iterator. If a parsing error is encountered, throws a 
    [ser_error](ser_error.md).

(3) Constructs a `staj_object_iterator` that refers to the first member of the object
    following the current stream event `begin_object`. If there is no such member,
    returns the end iterator. If a parsing error is encountered, returns the end iterator 
    and sets `ec`.

(4) Copy constructor

#### Member functions

    const key_value_type& operator*() const

    const key_value_type* operator->() const

    staj_object_iterator& operator++();
    staj_object_iterator operator++(int); 
    staj_object_iterator& increment(std::error_code& ec);
Advances the iterator to the next object member.

#### Non-member functions

    template <typename Key, typename T, typename CharT>
    staj_object_iterator<Key, T, CharT> begin(staj_object_iterator<Key, T, CharT> iter);   (1)

    template <typename T, typename CharT>
    staj_object_iterator<Key, T, CharT> end(staj_object_iterator<Key, T, CharT>) noexcept; (2)

    template <typename Key,typename T,typename CharT>
    bool operator==(const staj_object_iterator<Key, T, CharT>& a, 
        const staj_object_iterator<Key, T, CharT>& b)                                      (3)

    template <typename Key,typename T,typename CharT>
    bool operator!=(const staj_object_iterator<Key, T, CharT>& a,                          (4)
        const staj_object_iterator<Key, T, CharT>& b)

(1)-(2) For range-based for loop support.

(3)-(4) As required by LegacyInputIterator

### Examples

#### Iterate over a JSON object, returning key-json value pairs

```cpp
const std::string example = R"(
{
   "application": "hiking",
   "reputons": [
   {
       "rater": "HikingAsylum.array_example.com",
       "assertion": "advanced",
       "rated": "Marilyn C",
       "rating": 0.90
     }
   ]
}
)";

int main()
{
    json_string_cursor cursor(example);

    auto iter = staj_object_iterator<std::string,json>(cursor);

    for (const auto& kv : iter)
    {
        std::cout << kv.first << ":\n" << pretty_print(kv.second) << "\n";
    }
    std::cout << "\n\n";
}
```
Output:
```
application:
"hiking"
reputons:
[
    {
        "assertion": "advanced",
        "rated": "Marilyn C",
        "rater": "HikingAsylum.array_example.com",
        "rating": 0.90
    }
]
```

