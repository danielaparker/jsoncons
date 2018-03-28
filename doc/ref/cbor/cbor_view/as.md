### jsoncons::cbors::cbor_view::as

```c++
bool as_bool() const; // (1)

int64_t as_integer() const; // (2)

uint64_t as_uinteger() const; // (3)

double as_double() const; // (4)

string_type as_string() const; // (5)
```

(1) Returns `true` if value is `bool` and `true`, `false` if value is `bool` and `false`.
Otherwise throws `std::runtime_exception`.

(2) Returns integer value if value is integral and within range, otherwise throws `std::runtime_exception`.

(3) Returns integer value if value is integral and unsigned, otherwise throws `std::runtime_exception`.

(4) Returns floating point value if value is floating point, value cast to double if value is integral, otherwise throws `std::runtime_exception`.

(5) Returns value if value is string, otherwise throws `std::runtime_exception`.

