### jsoncons::cbors::cbor_view::is

```c++
bool is_null() const; // (1)

bool is_string() const; // (2)

bool is_byte_string() const; // (3)

bool is_integer() const; // (4)

bool is_uinteger() const; // (5)

bool is_double() const; // (6)

bool is_bool() const; // (7)

bool is_array() const; // (8)

bool is_object() const; // (9)
```

(1) Returns `true` if the first byte in the CBOR buffer is a null tag, otherwise `false`.

(2) Returns `true` if the first byte in the CBOR buffer is a text string tag, otherwise `false`.

(3) Returns `true` if the first byte in the CBOR buffer is a byte string tag, otherwise `false`.

(4) Returns `true` if the first byte in the CBOR buffer is a tag compatible with int64_t, otherwise `false`.

(5) Returns `true` if the first byte in the CBOR buffer is a tag compatible with uint84_t, otherwise `false`.

(6) Returns `true` if the first byte in the CBOR buffer is a tag that represents a floating point, otherwise `false`.

(7) Returns `true` if the first byte in the CBOR buffer is a boolean tag, otherwise `false`.

(8) Returns `true` if the first byte in the CBOR buffer is a tag for a CBOR array, otherwise `false`.

(9) Returns `true` if the first byte in the CBOR buffer is a tag for a CBOR map, otherwise `false`.



