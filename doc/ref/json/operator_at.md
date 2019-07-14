### jsoncons::basic_json::operator[]

```c++
proxy_type operator[](const string_view_type& key); // (1)

const_reference operator[](const string_view_type& key) const; // (2)

reference operator[](size_t i); // (3)

const_reference operator[](size_t i) const; // (4)
```

(1) Returns a proxy with underlying type the keyed value. Type `proxy_type` supports conversion to `basic_json&`. If written to, 
inserts or updates with the new value. If read, evaluates to a reference to the keyed value, if it exists, otherwise throws. 
Throws `std::runtime_error` if not an object.
If read, throws `std::out_of_range` if the object does not have a member with the specified name.  

(2) If `key` matches the key of a member in the basic_json object, returns a reference to the basic_json object, otherwise throws.
Throws `std::runtime_error` if not an object.
Throws `std::out_of_range` if the object does not have a member with the specified name.  

(3) Returns a reference to the value at index i in a `basic_json` object or array.
Throws `std::runtime_error` if not an object or array.

(4) Returns a `const_reference` to the value at index i in a `basic_json` object or array.
Throws `std::runtime_error` if not an object or array.

#### Notes

Unlike `std::map::operator[]`, a new element is never inserted into the container 
when this operator is used for reading but the key does not exist.


