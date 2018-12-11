### jsoncons::json::insert

```c++
template <class T>
array_iterator insert(const_array_iterator pos, T&& val); // (1)

template <class InputIt>
array_iterator insert(const_array_iterator pos, InputIt first, InputIt last); // (2)
```
(1) Adds a new json element at the specified position of a json array, shifting all elements currently at or above that position to the right.
The argument `val` is forwarded to the `json` constructor as `std::forward<T>(val)`.
Returns an `array_iterator` that points to the new value
Throws `std::runtime_error` if not an array.  

(2) Inserts elements from range [first, last) before pos.

#### See also

- [push_back](json/push_back.md)

### Examples

#### Creating an array of elements 
```c++
json cities = json::array();       // an empty array
std::cout << cities << std::endl;  // output is "[]"

cities.push_back("Toronto");  
cities.push_back("Vancouver");
// Insert "Montreal" at beginning of array
cities.insert(cities.array_range().begin(),"Montreal");  

std::cout << cities << std::endl;
```
Output:
```
[]
["Montreal","Toronto","Vancouver"]
```
#### Creating an array of elements with reserved storage 
```c++
json cities = json::array();  
cities.reserve(10);  // storage is reserved
std::cout << "capacity=" << cities.capacity() 
          << ", size=" << cities.size() << std::endl;

cities.push_back("Toronto");  
cities.push_back("Vancouver");
cities.insert(cities.array_range().begin(),"Montreal");
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

