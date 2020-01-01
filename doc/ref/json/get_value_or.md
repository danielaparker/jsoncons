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
Throws `std::domain_error` if not an object or null value.

### Examples

#### Returning a value or default

```c++
#include <jsoncons/json.hpp>

int main()
{
    json j(json_object_arg, {{"author","Evelyn Waugh"},{"title","Sword of Honour"}});

    std::cout << "(1) " << j["author"].as<std::string>() << "\n";
    std::cout << "(2) " << j.at("title").as<std::string>() << "\n";
    std::cout << "(3) " << j.at_or_null("category").as<std::string>() << "\n";
    std::cout << "(4) " << j.get_value_or<std::string>("category","fiction") << "\n";
}
```
Output:
```
(1) Evelyn Waugh
(2) Sword of Honour
(3) null
(4) fiction
```
