### jsoncons::staj_object_iterator

```c++
template<class Json, class T = Json>
class staj_object_iterator
```

#### Header
```c++
#include <jsoncons/staj_reader.hpp>
```

A `staj_object_iterator` is an [InputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) that
accesses the individual stream events from a [staj_reader](staj_reader.md) and, provided that when it is constructed 
the current stream event has type `staj_event_type::begin_object`, it retrieves the elements 
of the JSON object as items of type `std::pair<string_type,T>`. If when it is constructed the current stream event does not have type 
`staj_event_type::begin_object`, it becomes equal to the default-constructed iterator.

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|`Json::char_type char_type`
`key_type`|`std::basic_string<char_type>`
`value_type`|`std::pair<string_type,T>`
`difference_type`|`std::ptrdiff_t`
`pointer`|`value_type*`
`reference`|`value_type&`
`iterator_category`|[std::input_iterator_tag](https://en.cppreference.com/w/cpp/iterator/iterator_tags)

#### Constructors

    staj_object_iterator()

    staj_object_iterator(basic_staj_reader<char_type>& reader)

#### Member functions

    const key_value_type& operator*() const

    const key_value_type* operator->() const

    staj_object_iterator& operator++()

    staj_object_iterator operator++(int) 

#### Non-member functions

    template <class Json, class T>
    bool operator==(const staj_object_iterator<Json, T>& a, const staj_object_iterator<Json, T>& b)

    template <class Json, class T>
    bool operator!=(const staj_object_iterator<Json, T>& a, const staj_object_iterator<Json, T>& b)

    template <class Json, class T>
    staj_object_iterator<Json, T> begin(staj_object_iterator<Json, T> iter) noexcept; // (1)

    template <class Json, class T>
    staj_object_iterator<Json, T> end(const staj_object_iterator<Json, T>&) noexcept; // (2)

(1) Returns iter unchanged

(2) Returns a default-constructed `stax_array_iterator`, which serves as an end iterator. The argument is ignored.

The `begin` and `end` non-member functions enable the use of `stax_array_iterators` with range-based for loops.

