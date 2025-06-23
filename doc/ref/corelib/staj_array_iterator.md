### jsoncons::staj_array_iterator

```cpp
#include <jsoncons/staj_iterator.hpp>

template<
    typename T,
    typename CharT=char
    >
class staj_array_iterator
```

A `staj_array_iterator` is an [InputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) that
accesses the individual stream events from a [staj_cursor](staj_cursor.md) and, provided that when it is constructed
the current stream event has type `staj_event_type::begin_array`, it retrieves the elements of the JSON array
as items of type `T`. If when it is constructed the current stream event does not have type `staj_event_type::begin_array`,
it becomes equal to the default-constructed iterator.

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|CharT
`value_type`|`T`
`difference_type`|`std::ptrdiff_t`
`pointer`|`value_type*`
`reference`|`value_type&`
`iterator_category`|[std::input_iterator_tag](https://en.cppreference.com/w/cpp/iterator/iterator_tags)

#### Constructors

    staj_array_iterator() noexcept;                              (1)

    staj_array_iterator(basic_staj_cursor<char_type>& cursor);   (2)

    staj_array_iterator(basic_staj_cursor<char_type>& cursor,   
        std::error_code& ec);                                    (3)

    staj_array_iterator(const staj_array_iterator& iter);        (4)

(1) Constructs the end iterator

(2) Constructs a `staj_array_iterator` that refers to the first element of the array
    following the current stream event `begin_array`. If there is no such element,
    returns the end iterator. If a parsing error is encountered, throws a 
    [ser_error](ser_error.md).

(3) Constructs a `staj_array_iterator` that refers to the first member of the array
    following the current stream event `begin_array`. If there is no such element,
    returns the end iterator. If a parsing error is encountered, returns the end iterator 
    and sets `ec`.

(4) Copy constructor

#### Member functions

    const T& operator*() const

    const T* operator->() const

    staj_array_iterator& operator++()
    staj_array_iterator& increment(std::error_code& ec)
    staj_array_iterator operator++(int) 
Advances the iterator to the next array element.

#### Non-member functions

    template <typename T,typename CharT>
    staj_array_iterator<T,CharT> begin(staj_array_iterator<T,CharT> iter);    (1)

    template <typename T,typename CharT>
    staj_array_iterator<T,CharT> end(staj_array_iterator<T,CharT>) noexcept;  (2)

    template <typename T, typename CharT>
    bool operator==(const staj_array_iterator<T, CharT>& a,                   (3)
        const staj_array_iterator<T, CharT>& b);

    template <typename T, typename CharT>
    bool operator!=(const staj_array_iterator<T, CharT>& a,                   (4)  
        const staj_array_iterator<T, CharT>& b);

(1)-(2) For range-based for loop support.

(3)-(4) As required by LegacyInputIterator

### Examples

#### Iterate over a JSON array, returning json values  

```cpp
const std::string example = R"(
[ 
  { 
      "employeeNo" : "101",
      "name" : "Tommy Cochrane",
      "title" : "Supervisor"
  },
  { 
      "employeeNo" : "102",
      "name" : "Bill Skeleton",
      "title" : "Line manager"
  }
]
)";

int main()
{
    std::istringstream is(example);

    json_stream_cursor cursor(is);

    auto iter = staj_array_iterator<json>(cursor);

    for (const auto& j : iter)
    {
        std::cout << pretty_print(j) << "\n";
    }
    std::cout << "\n\n";
}
```
Output:
```
{
    "employeeNo": "101",
    "name": "Tommy Cochrane",
    "title": "Supervisor"
}
{
    "employeeNo": "102",
    "name": "Bill Skeleton",
    "title": "Line manager"
}
```

#### Iterate over the JSON array, returning employee values 

```cpp
namespace ns {

    struct employee
    {
        std::string employeeNo;
        std::string name;
        std::string title;
    };

} // namespace ns

JSONCONS_ALL_MEMBER_TRAITS(ns::employee, employeeNo, name, title)
      
int main()
{
    std::istringstream is(example);

    json_stream_cursor cursor(is);

    auto iter = staj_array_iterator<ns::employee>(cursor);

    for (const auto& val : iter)
    {
        std::cout << val.employeeNo << ", " << val.name << ", " << val.title << "\n";
    }
    std::cout << "\n\n";
}
```
Output:
```
101, Tommy Cochrane, Supervisor
102, Bill Skeleton, Line manager
```

#### Non-throwing overloads

```cpp
#include <jsoncons/json.hpp>
#include <iostream>

namespace ns {

struct employee
{
    std::string name;
    uint64_t id;
    int age;
};

} // namespace ns


JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::employee,
    (name, "Name"),
    (id, "Id"),
    (age, "Age", JSONCONS_RDWR,
        [](int age) noexcept
        {
            return age >= 16 && age <= 68;
        }
        )
)

int main()
{
    const std::string input = R"(
    [
      {
        "Name" : "John Smith",
        "Id" : 22,
        "Age" : 345
      },
      {
        "Name" : "",
        "Id" : 23,
        "Age" : 36
      },
      {
        "Name" : "Jane Doe",
        "Id" : 24,
        "Age" : 34
      }
    ]
    )";

    std::error_code ec;
    jsoncons::json_string_cursor cursor(input, ec);

    auto iter = jsoncons::staj_array_iterator<ns::employee>(cursor, ec);
    auto last = end(iter);

    while (iter != last)
    {
        if (ec)
        {
            std::cout << "Fail: " << ec.message() << "\n";
        }
        else
        {
            std::cout << "id: " << iter->id 
                      << ", name: " << iter->name 
                      << ", age: " << iter->age << "\n";
        }
        iter.increment(ec);
    }
}
```
Output:
```
Fail: Unable to convert into the provided type
id: 23, name: , age: 36
id: 24, name: Jane Doe, age: 34
```
