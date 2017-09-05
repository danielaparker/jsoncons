### `jsoncons::json::json`

```c++
json(); // (1)

json(const allocator_type& allocator); // (2)

json(std::initializer_list<json> list, const allocator_type& allocator); // (3)

json(const json& val); // (4)

json(const json& val, const allocator_type& allocator); // (5)

json(json&& val) noexcept; // (6)

json(json&& val, const allocator_type& allocator); // (7)

template <class T>
json(const T& val); // (8)

template <class T>
json(const T& val, const allocator_type& allocator); // (9)

json(double val, uint8_t precision); // (10)

template <class T>
json(const char* val); // (11)

template <class T>
json(const char* val, const allocator_type& allocator); // (12)
```

(1) Constructs an empty json object. 

(2) Constructs a json object. 

(3) Constructs a json array with the elements of the initializer-list `init`. 

(4) Constructs a copy of val

(5) Copy with allocator

(6) Acquires the contents of val, leaving val a `null` value

(7) Move with allocator

(8) Constructs a `json` value for types supported in [json_type_traits](json_type_traits.md).

(9) Constructs a `json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(10) Constructs a `json` value for double value with specified precision.

(11) Constructs a `json` value for character string.

(12) Constructs a json value for character string with supplied allocator.

