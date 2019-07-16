### jsoncons::basic_json::at

```c++
basic_json& at(const string_view_type& name); // (1)

const basic_json& at(const string_view_type& name) const; // (2)

basic_json& at(size_t i); // (3)

const basic_json& at(size_t i) const; // (4)
```

(1)-(2) return a reference to the value with the specifed name in a 
basic_json object. If not an object, an exception of type
`std::runtime_error` is thrown. if the object does not have a 
member with the specified name, an exception of type
`std::out_of_range` is thrown. 

(3)-(4) return a reference to the element at index `i` in a 
basic_json array. If not an array, an exception of type
`std::runtime_error` is thrown. if the index is outside the 
bounds of the array, an exception of type `std::out_of_range`
is thrown.  

