### jsoncons::staj_object_iterator

```c++
template<class Json, class T = Json>
class staj_object_iterator
```

#### Header
```c++
#include <jsoncons/staj_reader.hpp>
```
#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|`Json::char_type char_type`
`key_type`|`std::basic_string<char_type>`
`value_type`|`std::pair<string_type,T>`
`difference_type`|`std::ptrdiff_t`
`pointer`|`value_type*`
`reference`|`value_type&`

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
    staj_object_iterator<Json, T> begin(staj_object_iterator<Json, T> iter) noexcept

    template <class Json, class T>
    staj_object_iterator<Json, T> end(const staj_object_iterator<Json, T>&) noexcept

