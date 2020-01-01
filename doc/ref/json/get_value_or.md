### jsoncons::basic_json::get_value_or

```c++
template <class T,class U>
T get_value_or(const string_view_type& name, U&& v) const &; // (1)

template <class T,class U>
T get_value_or(const string_view_type& name, U&& v) const &&; // (2)
```

(1)-(2) return a value in a basic_json object
if `name` matches the name of a member, 
otherwise return a default value.
Throws `std::runtime_error` if not an object or null value.

### Examples

#### Returning a value or default

```c++
#include <jsoncons/json.hpp>

int main()
{
    json j(json_object_arg, {{"author","Evelyn Waugh"},{"title","Sword of Honour"}});

    std::cout << j.at_or_null("author").as<std::string>() << "\n";
    std::cout << j.at_or_null("category").as<std::string>() << "\n";
}
```
Output:
```
Evelyn Waugh
null
```
