### jsoncons::basic_json::as_or

```cpp
template <typename T,typename U>
T as_or(U&& default_value) const; 
```

Checks the value of the json value with the template value using [json_type_traits](../json_type_traits.md), and converts if it matches, or returns a default value if it doesn't.

See [as](as.md) for details on the value returned.

### Type requirements

- `U` must be convertible to `T`

### Examples

#### Return a value if the type matches, or a default value otherwise

```cpp
#include <jsoncons/json.hpp>

int main()
{
    json j = json::parse("\"Hello World\"");

    std::cout << j.as_or<int>(-1) << "\n";
    std::cout << j.as_or<std::string>("null") << "\n";
}
```
Output:
```
-1
Hello World
```

#### Return a value if it exists and the type matches, or a default value otherwise

```cpp
#include <jsoncons/json.hpp>

int main()
{
    json j(json_object_arg, {{"author","Evelyn Waugh"},{"title","Sword of Honour"},{"id","0"}});
    std::cout << j.at_or_null("author").as_or<std::string>("unknown") << "\n";
    std::cout << j.at_or_null("category").as_or<std::string>("fiction") << "\n";
    std::cout << j.at_or_null("id").as_or<int>(-1) << "\n";
}
```
Output:
```
Evelyn Waugh
fiction
-1
```
