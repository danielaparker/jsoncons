### jsoncons::json::is

```c++
template <class T, class... Args>
bool is(Args&&... args) const noexcept
```
Returns `true` if the json value is the same as type `T` according to [json_type_traits](json_type_traits.md), `false` otherwise.  

    bool is<X> const noexcept 
Type `X` is integral: returns `true` if the json value is integral and within the range of the type `X`, `false` otherwise.  
Type `X` is floating point: returns `true` if the json value is floating point and within the range of the type `X`, `false` otherwise.  

    bool is<std::string> const noexcept 
Returns `true` if the json value is of string type, `false` otherwise.  

    bool is<bool> const noexcept 
Returns `true` if the json value is of boolean type, `false` otherwise.  

    bool is<json::null_type> const noexcept
Returns `true` if the json value is null, `false` otherwise.  

    bool is<json::object> const noexcept 
Returns `true` if the json value is an object, `false` otherwise.  

    bool is<json::array> const noexcept 
Returns `true` if the json value is an array, `false` otherwise.  

    bool is<X<T>> const noexcept
If the type `X` is not `std::basic_string` but otherwise satisfies [SequenceContainer](http://en.cppreference.com/w/cpp/concept/SequenceContainer), `is<X<T>>()` returns `true` if the json value is an array and each element is the "same as" type `T` according to [json_type_traits](json_type_traits.md), `false` otherwise.

    bool is<X<std::string,T>> const noexcept
If the type 'X' satisfies [AssociativeContainer](http://en.cppreference.com/w/cpp/concept/AssociativeContainer) or [UnorderedAssociativeContainer](http://en.cppreference.com/w/cpp/concept/UnorderedAssociativeContainer), `is<X<T>>()` returns `true` if the json value is an object and each mapped value is the "same as" `T` according to [json_type_traits](json_type_traits.md), `false` otherwise.


### Examples

```c++
json j = json::parse(R"(
{
    "k1" : 2147483647,
    "k2" : 2147483648,
    "k3" : -10,
    "k4" : 10.5,
    "k5" : true,
    "k6" : "10.5"
}
)");

std::cout << std::boolalpha << "(1) " << j["k1"].is<int32_t>() << '\n';
std::cout << std::boolalpha << "(2) " << j["k2"].is<int32_t>() << '\n';
std::cout << std::boolalpha << "(3) " << j["k2"].is<long long>() << '\n';
std::cout << std::boolalpha << "(4) " << j["k3"].is<signed char>() << '\n';
std::cout << std::boolalpha << "(5) " << j["k3"].is<uint32_t>() << '\n';
std::cout << std::boolalpha << "(6) " << j["k4"].is<int32_t>() << '\n';
std::cout << std::boolalpha << "(7) " << j["k4"].is<double>() << '\n';
std::cout << std::boolalpha << "(8) " << j["k5"].is<int>() << '\n';
std::cout << std::boolalpha << "(9) " << j["k5"].is<bool>() << '\n';
std::cout << std::boolalpha << "(10) " << j["k6"].is<double>() << '\n';

```
Output:
```
(1) true
(2) false
(3) true
(4) true
(5) false
(6) false
(7) true
(8) false
(9) true
(10) false


