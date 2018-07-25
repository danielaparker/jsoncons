### jsoncons::cbor::cbor_view::is

```c++
template <class T, class... Args>
bool is(Args&&... args) const noexcept; // (1)

bool is_null() const; // (2)

bool is_string() const; // (3)

bool is_byte_string() const; // (4)

bool is_integer() const; // (5)

bool is_uinteger() const; // (6)

bool is_double() const; // (7)

bool is_bool() const; // (8)

bool is_byte_string() const; // (9)

bool is_bignum() const; // (10)

bool is_array() const; // (11)

bool is_object() const; // (12)
```

(2) Returns `true` if the first byte in the CBOR buffer is a null tag, otherwise `false`.

(3) Returns `true` if the first byte in the CBOR buffer is a text string tag, otherwise `false`.

(4) Returns `true` if the first byte in the CBOR buffer is a byte string tag, otherwise `false`.

(5) Returns `true` if the first byte in the CBOR buffer is a tag compatible with int64_t, otherwise `false`.

(6) Returns `true` if the first byte in the CBOR buffer is a tag compatible with uint84_t, otherwise `false`.

(7) Returns `true` if the first byte in the CBOR buffer is a tag that represents a floating point, otherwise `false`.

(8) Returns `true` if the first byte in the CBOR buffer is a boolean tag, otherwise `false`.

(11) Returns `true` if the first byte in the CBOR buffer is a tag for a CBOR array, otherwise `false`.

(12) Returns `true` if the first byte in the CBOR buffer is a tag for a CBOR map, otherwise `false`.



