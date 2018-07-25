### jsoncons::cbor::cbor_view::as

```c++
template <class T, class... Args>
T as(Args&&... args) const; // (1)

bool as_bool() const; // (2)

int64_t as_integer() const; // (3)

uint64_t as_uinteger() const; // (4)

double as_double() const; // (5)

std::string as_string() const; // (6)

byte_string as_byte_string() const; // (7)

bignum as_bignum() const; // (8)
```

(2) Returns `true` if value is `bool` and `true`, `false` if value is `bool` and `false`.
Otherwise throws `std::runtime_exception`.

(3) Returns integer value if value is integral and within range, otherwise throws `std::runtime_exception`.

(4) Returns integer value if value is integral and unsigned, otherwise throws `std::runtime_exception`.

(5) Returns floating point value if value is floating point, value cast to double if value is integral, otherwise throws `std::runtime_exception`.

(6) Returns value if value is string, otherwise converts value to string.

