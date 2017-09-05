### `jsoncons::json::json`

```c++
json(); // (1)

json(const allocator_type& allocator); // (2)

json(std::initializer_list<json> list, const allocator_type& allocator); // (3)

json(const json& val); // (4)

json(const json& val, const allocator_type& allocator); // (5)

json(json&& val) noexcept; // (6)

json(json&& val, const allocator_type& allocator) noexcept; // (7)

json(const array& val); // (8)

json(array&& val) noexcept; // (9)

json(const object& val); // (10)

json(object&& val) noexcept; // (11)

template <class T>
json(const T& val); // (12)

template <class T>
json(const T& val, const allocator_type& allocator); // (13)

json(double val, uint8_t precision); // (14)

json(const char* val); // (15)

json(const char* val, const allocator_type& allocator); // (16)
```

(1) Constructs a `json` value that holds an empty json object. 

(2) Constructs a `json` value that holds a json object. 

(3) Constructs a `json` array with the elements of the initializer-list `init`. 

(4) Constructs a copy of val

(5) Copy with allocator

(6) Acquires the contents of val, leaving val a `null` value

(7) Move with allocator

(8) Constructs a `json` value from a json array

(9) Acquires the contents of a json array

(10) Constructs a `json` value from a json object

(11) Acquires the contents of a json object

(12) Constructs a `json` value for types supported in [json_type_traits](json_type_traits.md).

(13) Constructs a `json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(14) Constructs a `json` value for double value with specified precision.

(15) Constructs a `json` value for character string.

(16) Constructs a `json` value for character string with supplied allocator.

