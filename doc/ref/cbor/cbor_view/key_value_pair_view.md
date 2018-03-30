### jsoncons::cbor::key_value_pair_view 

```c++
template <class T>
class key_value_pair_view
```

A `key_value_pair_view` refers to a contiguous sequence of bytes representing a CBOR map key and a contiguous sequence of bytes representing a CBOR map value.
It holds pointers to the beginning and end of a CBOR key, and pointers to the beginning and end of a CBOR value.

#### Accessors
    
    std::string key() const

    T value() const


