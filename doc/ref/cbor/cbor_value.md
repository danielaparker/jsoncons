### jsoncons::cbor::cbor_value

Wraps a packed [cbor](http://cbor.io/) value.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

class cbor_value
```

Member type                         |Definition
------------------------------------|------------------------------
`value_type`|`cbor_view`
`const_reference`|`const cbor_view&`

#### Constructors

```c++
cbor_value(); // (1)

cbor_value(const cbor_value& val); // (2)

cbor_value(cbor_value&& val); // (3)

cbor_value(const std::vector<uint8_t>& v); // (4)

cbor_value(std::vector<uint8_t>&& v); // (5)

template< class InputIt >
cbor_value(InputIt first, InputIt last); // (6)

cbor_value(cbor_view val); // (7)

```

#### CBOR buffer

<table border="0">
  <tr>
    <td><code>const uint8_t* buffer() const</code></td>
    <td>Returns a pointer to the first byte of the CBOR buffer.</td> 
  </tr>
  <tr>
    <td><code>size_t buflen() const</code></td>
    <td>Returns length of CBOR buffer.</td> 
  </tr>
</table>

#### Accessors

<table border="0">
  <tr>
    <td><code>bool is_array() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a CBOR tag that indicates an array, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>bool is_object() const</code></td>
    <td>Returns <code>true</code> if the first byte in the CBOR buffer is a CBOR tag that indicates a map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>size_t size() const</code></td>
    <td>Returns the length of the array or map if the first byte in the CBOR buffer is a CBOR tag that indicates an array or map, otherwise <code>false</code>.</td> 
  </tr>
  <tr>
    <td><code>cbor_view at(size_t pos) const</code></td>
    <td>Returns a view of the CBOR array element at specified index <code>pos</code>.</td> 
  </tr>
  <tr>
    <td><code>cbor_view at(const std::string& key) const</code></td>
    <td>Returns a view of the CBOR object member value with key equivalent to <code>key</code>.</td> 
  </tr>
</table>

#### See also

- [cbor_view](cbor_view.md)


