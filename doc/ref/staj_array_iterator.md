### jsoncons::staj_array_iterator

```c++
template<class Json, class T = Json>
class staj_array_iterator
```

#### Header
```c++
#include <jsoncons/staj_reader.hpp>
```
#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|`Json::char_type char_type`
`value_type`|`T`
`difference_type`|`std::ptrdiff_t`
`pointer`|`value_type*`
`reference`|`value_type&`

#### Constructors

    staj_array_iterator()

    staj_array_iterator(basic_staj_reader<char_type>& reader)

#### Member functions

    const T& operator*() const

    const T* operator->() const

    staj_array_iterator& operator++()

    staj_array_iterator operator++(int) 

#### Non-member functions

    template <class Json, class T>
    bool operator==(const staj_array_iterator<Json, T>& a, const staj_array_iterator<Json, T>& b)

    template <class Json, class T>
    bool operator!=(const staj_array_iterator<Json, T>& a, const staj_array_iterator<Json, T>& b)

    template <class Json, class T>
    staj_array_iterator<Json, T> begin(staj_array_iterator<Json, T> iter) noexcept

    template <class Json, class T>
    staj_array_iterator<Json, T> end(const staj_array_iterator<Json, T>&) noexcept

### Examples

#### Iterate over a JSON array, , returning `json` values  

```c++
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

    json_stream_reader reader(is);

    staj_array_iterator<json> it(reader);

    for (const auto& j : it)
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


#### Iterate over the JSON array, returning `employee` values 

```c++
struct employee
{
    std::string employeeNo;
    std::string name;
    std::string title;
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, employee>
    {
    template<class Json>
    struct json_type_traits<Json, employee>
    {
        static bool is(const Json& j) noexcept
        {
            return j.is_object() && j.contains("employeeNo") && j.contains("name") && j.contains("title");
        }
        static employee as(const Json& j)
        {
            employee val;
            val.employeeNo = j["employeeNo"].template as<std::string>();
            val.name = j["name"].template as<std::string>();
            val.title = j["title"].template as<std::string>();
            return val;
        }
        static Json to_json(const employee& val)
        {
            Json j;
            j["employeeNo"] = val.employeeNo;
            j["name"] = val.name;
            j["title"] = val.title;
            return j;
        }
    };
}  
      
int main()
{
    std::istringstream is(example);

    json_stream_reader reader(is);

    staj_array_iterator<json,employee> it(reader);

    for (const auto& val : it)
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

