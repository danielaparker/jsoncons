### jsoncons::cbor::ondemand::cbor_item

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
```

An `cbor_item` is one complete, structurally valid encoding. 

#### Accessors

<table border="0">
  <tr>
    <td>cbor_major_type major_type() const noexcept</td>
    <td>Returns a <a href="../cbor_major_type.md">cbor_major_type</></td> 
  </tr>
  <tr>
    <td>uint8_t additional_info() const noexcept</td>
    <td>Returns CBOR additional information.</td> 
  </tr>
  <tr>
    <td>cbor_type type() const noexcept</td>
    <td>Returns a <a href="cbor_type.md">cbor_type</a>.</td> 
  </tr>
  <tr>
    <td>uint64_t argument() const noexcept</td>
    <td>Returns integer's value, a string's length, a container's count,
or a simple value's number (the bit pattern of a float).</td> 
  </tr>
  <tr>
    <td>bool indefinite() const noexcept</td>
    <td>Returns true if a byte string or text string has indefinite length, otherwise false.</td> 
  </tr>
  <tr>
    <td>tag_range tags() const noexcept</td>
    <td>Returns the the leading tag numbers, outermost first.</td> 
  </tr>
  <tr>
    <td>child_range children() const noexcept</td>
    <td>Returns the data items an item is made of, as checked items: an
array's elements, a map's keys and values alternating, or the definite-length
chunks of an indefinite-length string (RFC 8949 section 3.2.3). It is empty
for anything else. Each child is measured once as the iteration reaches it;
the last child of a definite-length container is not measured at all, since
it ends where its parent does.</td> 
  </tr>
  <tr>
    <td>access_result&lt;uint64_t&gt; get_uint64() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;uint64_t&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;int64_t&gt; get_int64() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;int64_t&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;bool> get_bool() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;bool&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;double> get_double() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;double&gt;</a>.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::string_view> get_string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a>
    guaranteed to contain valid UTF-8.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::u8string_view> get_u8string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::string_view&gt;</a> 
    guaranteed to contain valid UTF-8. This function template participates in overload resolution 
    only if **__cpp_char8_t** is defined.</td> 
  </tr>
  <tr>
    <td>access_result&lt;jsoncons::span&lt;const uint8_t&gt;> get_byte_string() const noexcept</td>
    <td>Returns an <a href="access_result.md">access_result&lt;jsoncons::span&lt;const uint8_t&gt;&gt;</a>.</td> 
  </tr>
  <tr>
    <td>template <typename CharBuffer><br/> access_result&lt;void> get_string(CharBuffer& buffer) const</td>
    <td>Copying get_string overload guaranteed to produce valid UTF-8, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
  <tr>
    <td>template <typename ByteBuffer><br/>access_result&lt;void> get_byte_string(ByteBuffer& buffer) const</td>
    <td>Copying get_byte_string overload, assembles
indefinite-length strings into the provided buffer.</td> 
  </tr>
</table>

#### Non-member functions

```cpp
bool validate_text(const cbor_item&) noexcept;
```

`validate_text` checks that a text item is well-formed UTF-8. Each chunk of an
indefinite-length text string is checked independently, as RFC 8949 section
3.2.3 requires.

