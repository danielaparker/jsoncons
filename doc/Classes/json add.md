    template <class T>
    void add(T&& val)
Adds a new json element at the end of a json array. The argument `val` is forwarded to the `json` constructor as `std::forward<T>(val)`.
Throws `std::runtime_error` if not an array.

    template <class T>
    array_iterator add(const_array_iterator pos, T&& val)
Adds a new json element at the specified position of a json array, shifting all elements currently at or above that position to the right.
The argument `val` is forwarded to the `json` constructor as `std::forward<T>(val)`.
Returns an `array_iterator` that points to the new value
Throws `std::runtime_error` if not an array.

## Examples

### Creating an array of elements 
```c++
json cities = json::array();       // an empty array
std::cout << cities << std::endl;  // output is "[]"

cities.add("Toronto");  
cities.add("Vancouver");
// Insert "Montreal" at beginning of array
cities.add(cities.array_range().begin(),"Montreal");  

std::cout << cities << std::endl;
```
Output:
```
[]
["Montreal","Toronto","Vancouver"]
```
### Creating an array of elements with reserved storage 
```c++
json cities = json::array();  
cities.reserve(10);  // storage is reserved
std::cout << "capacity=" << cities.capacity() 
          << ", size=" << cities.size() << std::endl;

cities.add("Toronto");  
cities.add("Vancouver");
cities.add(cities.array_range().begin(),"Montreal");
std::cout << "capacity=" << cities.capacity() 
          << ", size=" << cities.size() << std::endl;

std::cout << cities << std::endl;
```
Output:
```
capacity=10, size=0
capacity=10, size=3
["Montreal","Toronto","Vancouver"]
```

